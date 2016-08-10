#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「トレーニング用スクリプト 3Dバージョン」
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
import nl3d


#### CONFIGURATION ####
DIR_DATA = './answer'
DIR_DUMP = './dump'

parser = argparse.ArgumentParser(description='Machine learning based nl-solver test: WINDOW (training)')
parser.add_argument('--size', '-s', default=3, type=int,
                    help='Window size (dimension)')
parser.add_argument('--unit', '-u', default=100, type=int,
                    help='Number of units in hidden layer')
parser.add_argument('--epoch', '-e', default=100000, type=int,
                    help='Number of epoches')
parser.add_argument('--test', '-t', default='none', type=str,
                    help='Filename of test (which will not be trained) or none (default: none)')
parser.add_argument('--show-wrong', '-w', default=False, action='store_true',
                    help='Set on to print incorrect lines in red (default: False)')
parser.add_argument('--dataset', '-d', default='window', type=str,
                    help='Input dataset type to ML: window (dafault), windowsn, windowxa, windowxb')
args = parser.parse_args()
print args

n_dims             = args.size
n_dims_half        = n_dims / 2
n_units            = args.unit
n_epoch            = args.epoch
testfilename       = args.test
testfilename_short = testfilename.replace('_', '')
if '.txt' in testfilename:
    testfilename_short = testfilename[0:-4]


# [3.1] 準備
# Prepare dataset --> nl.py


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 n_units 次元)
# 入力: N x N - 1 = N^2 - 1 次元 (dataset = window の場合)
#       N x N = N^2 次元         (dataset = windowsn の場合)
#       N x N = N^2 + 2 次元     (dataset = windowxa の場合)
#       N x N = N^2 + 1 次元     (dataset = windowxb の場合)
# 出力: 7次元
if args.dataset == 'window':
    input_dims = n_dims**2 - 1
elif args.dataset == 'windowsn':
    input_dims = n_dims**2
elif args.dataset == 'windowxa':
    input_dims = n_dims**2 + 2
elif args.dataset == 'windowxb':
    input_dims = n_dims**2 + 1
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
x_train_raw, y_train_raw = [], []
x_test_raw,  y_test_raw  = [], []

# トレーニング/テスト ファイルを読み込む
datafiles = glob.glob(DIR_DATA + '/*.txt')
for datafile in datafiles:
    path_datafile = datafile.replace('\\', '/').split('/')
    datafilename = path_datafile[-1]
    datafilename_woext = datafilename[0:-4]
    # Training data
    if datafilename != testfilename and datafilename_woext != testfilename:
        print 'Reading train file: {}/{} ...'.format(DIR_DATA, datafilename)
        _board_x, _board_y, _board_z, _boards = nl3d.read_ansfile(datafile, n_dims)

        for z in range(_board_z):
            x_data, y_data = nl.gen_dataset_shape(_board_x, _board_y, _boards[z], n_dims, args.dataset) # 配線形状の分類
            #x_data, y_data = nl.gen_dataset_dirsrc(board_x, board_y, boards[z], n_dims) # 配線接続位置の分類 (ソースから)
            #x_data, y_data = nl.gen_dataset_dirsnk(board_x, board_y, boards[z], n_dims) # 配線接続位置の分類 (シンクから)

            x_train_raw = x_train_raw + x_data
            y_train_raw = y_train_raw + y_data
    # Test data
    else:
        print 'Reading test file: {}/{} ...'.format(DIR_DATA, datafilename)
        board_x, board_y, board = nl.read_ansfile(datafile, n_dims)

        x_data, y_data = nl.gen_dataset_shape(board_x, board_y, board, n_dims, args.dataset) # 配線形状の分類
        #x_data, y_data = nl.gen_dataset_dirsrc(board_x, board_y, board, n_dims) # 配線接続位置の分類 (ソースから)
        #x_data, y_data = nl.gen_dataset_dirsnk(board_x, board_y, board, n_dims) # 配線接続位置の分類 (シンクから)

        x_test_raw = x_test_raw + x_data
        y_test_raw = y_test_raw + y_data
print ''

assert(len(x_train_raw) != 0)
assert(len(y_train_raw) != 0)
x_train = np.array(x_train_raw, dtype=np.float32)
y_train = np.array(y_train_raw, dtype=np.int32)

if testfilename != 'none':
    assert(len(x_test_raw) != 0)
    assert(len(y_test_raw) != 0)
    x_test = np.array(x_test_raw, dtype=np.float32)
    y_test = np.array(y_test_raw, dtype=np.int32)

for epoch in xrange(1, n_epoch + 1):
    # Training
    # バッチサイズごとに学習する方法もある
    # http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412

    # 順番をランダムに並び替える
    n_train = len(y_train)
    perm = np.random.permutation(n_train)

    # バッチサイズごとに学習する
    sum_loss = 0
    sum_accuracy = 0
    batchsize = 100
    for i in range(0, n_train, batchsize):
        x_batch = x_train[perm[i:i+batchsize]]
        y_batch = y_train[perm[i:i+batchsize]]

        # 勾配を初期化
        optimizer.zero_grads()

        # 順伝播させて誤差と精度を算出
        loss_train, accuracy_train, _ = forward(x_batch, y_batch)

        # 誤差逆伝播で勾配を計算
        loss_train.backward()
        optimizer.update()

        sum_loss     = sum_loss + loss_train.data * batchsize
        sum_accuracy = sum_accuracy + accuracy_train.data * batchsize

    # Evaluation
    if testfilename != 'none':
        loss_test, accuracy_test, result = forward(x_test, y_test, train=False)

    # 訓練データ/テストデータの誤差と、正解精度を表示
    if epoch % 100 == 0:
        print 'epoch', epoch
        print 'Train: mean loss={}, accuracy={}'.format(sum_loss / n_train, sum_accuracy / n_train)
        if testfilename != 'none':
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
with open(DIR_DUMP + '/s{}_u{}_e{}_d{}_t{}.pkl'.format(n_dims, n_units, n_epoch, args.dataset, testfilename_short), 'w') as f:
    pickle.dump(model, f)
