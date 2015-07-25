#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「トレーニング用スクリプト」
周囲 N x N = N^2 マスを切り出して (window) 学習してみる

参考文献:
* Python - 【機械学習】ディープラーニング フレームワークChainerを試しながら解説してみる。 - Qiita http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412
"""

import argparse
import glob
import pickle
import sys

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F

import nl


#### CONFIGURATION ####
parser = argparse.ArgumentParser(description='Machine learning based nl-solver test: WINDOW (training)')
parser.add_argument('--size', '-s', default=3, type=int,
                    help='Window size (dimension)')
parser.add_argument('--unit', '-u', default=100, type=int,
                    help='Number of units in hidden layer')
parser.add_argument('--epoch', '-e', default=100000, type=int,
                    help='Number of epoches')
parser.add_argument('--show-wrong', '-w', default=False, action='store_true',
                    help='Set on to print incorrect lines in red (default: False)')
parser.add_argument('--dataset', '-d', default='window', type=str,
                    help='Input dataset type to ML (dafault: window)')
args = parser.parse_args()
print args

n_dims            = args.size
n_dims_half       = n_dims / 2
n_units           = args.unit
n_epoch           = args.epoch


# [3.1] 準備
# Prepare dataset --> nl.py


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 n_units 次元)
# 入力: N x N - 1 = N^2 - 1 次元 (dataset = window の場合)
#       N x N = N^2 次元         (dataset = windowsn の場合)
# 出力: 7次元
if args.dataset == 'window':
    input_dims = n_dims**2 - 1
elif args.dataset == 'windowsn':
    input_dims = n_dims**2
else:
    raise NotImplementedError()

model = FunctionSet(l1=F.Linear(input_dims, n_units),
                    l2=F.Linear(n_units, n_units),
                    l3=F.Linear(n_units, 7))

# Neural net architecture
# ニューラルネットの構造
def forward(x_data, y_data, train=True):
    x, t = Variable(x_data), Variable(y_data)
    h1 = F.dropout(F.relu(model.l1(x)),  train=train)
    h2 = F.dropout(F.relu(model.l2(h1)), train=train)
    y  = model.l3(h2)
    # 多クラス分類なので誤差関数としてソフトマックス関数の
    # 交差エントロピー関数を用いて、誤差を導出
    return F.softmax_cross_entropy(y, t), F.accuracy(y, t), y


# [3.3] Optimizerの設定
# Setup optimizer
optimizer = optimizers.Adam()
optimizer.setup(model.collect_parameters())


# Learning loop
_x_train, _y_train = [], []
_x_test,  _y_test  = [], []

# 複数ファイルを読み込む
train_files = glob.glob('./data-train/*.txt')
test_files  = glob.glob('./data-test/*.txt')
assert(len(test_files) == 1)

for train_file in train_files:
    print 'Reading training file: {} ...'.format(train_file)
    board_x, board_y, board = nl.read_ansfile(train_file, n_dims)

    x_data, y_data = nl.gen_dataset_shape(board_x, board_y, board, n_dims, args.dataset) # 配線形状の分類
    #x_data, y_data = nl.gen_dataset_dirsrc(board_x, board_y, board, n_dims) # 配線接続位置の分類 (ソースから)
    #x_data, y_data = nl.gen_dataset_dirsnk(board_x, board_y, board, n_dims) # 配線接続位置の分類 (シンクから)

    _x_train = _x_train + x_data
    _y_train = _y_train + y_data

for test_file in test_files:
    print 'Reading testing file: {} ...'.format(test_file)
    board_x, board_y, board = nl.read_ansfile(test_file, n_dims)

    x_data, y_data = nl.gen_dataset_shape(board_x, board_y, board, n_dims, args.dataset) # 配線形状の分類
    #x_data, y_data = nl.gen_dataset_dirsrc(board_x, board_y, board, n_dims) # 配線接続位置の分類 (ソースから)
    #x_data, y_data = nl.gen_dataset_dirsnk(board_x, board_y, board, n_dims) # 配線接続位置の分類 (シンクから)

    _x_test = _x_test + x_data
    _y_test = _y_test + y_data

x_train = np.array(_x_train, dtype=np.float32)
y_train = np.array(_y_train, dtype=np.int32)

x_test = np.array(_x_test, dtype=np.float32)
y_test = np.array(_y_test, dtype=np.int32)

for epoch in xrange(1, n_epoch + 1):
    # Training
    # バッチサイズごとに学習する方法もある
    # http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412

    # 順番をランダムに並び替える
    n_train = len(y_train)
    perm = np.random.permutation(n_train)
    x_batch = x_train[perm[0:n_train]]
    y_batch = y_train[perm[0:n_train]]

    # 勾配を初期化
    optimizer.zero_grads()

    # 順伝播させて誤差と精度を算出
    loss_train, accuracy_train, _ = forward(x_batch, y_batch)

    # 誤差逆伝播で勾配を計算
    loss_train.backward()
    optimizer.update()

    # Evaluation
    loss_test, accuracy_test, result = forward(x_test, y_test, train=False)

    # 訓練データ/テストデータの誤差と、正解精度を表示
    if epoch % 100 == 0:
        print 'epoch', epoch
        print 'Train: mean loss={}, accuracy={}'.format(loss_train.data, accuracy_train.data)
        print 'Test:  mean loss={}, accuracy={}'.format(loss_test.data,  accuracy_test.data)

        # テストデータの配線を表示
        idx = 0
        str = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
        for y in range(n_dims_half, board_y + n_dims_half):
            for x in range(n_dims_half, board_x + n_dims_half):
                if board[y][x]['type'] == 1:
                    sys.stdout.write('\033[1;30;47m ' + nl.int2str(board[y][x]['data'], 36) + ' \033[0m')
                else:
                    ex_shape = np.argmax(result.data[idx])
                    # 正しい配線形状
                    if (not args.show_wrong) or board[y][x]['shape'] == ex_shape:
                        sys.stdout.write('\033[1;30;47m' + str[ex_shape] + '\033[0m')
                    # 間違ってる配線形状
                    else:
                        sys.stdout.write('\033[1;31;47m' + str[ex_shape] + '\033[0m')
                    idx = idx + 1
            print ''

# モデルをシリアライズ化して保存
with open('s{}_u{}_e{}_d{}.pkl'.format(n_dims, n_units, n_epoch, args.dataset), 'w') as f:
    pickle.dump(model, f)
