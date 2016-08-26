#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「トレーニング用スクリプト 3D 2016年バージョン」

アイデア:
セルそのものを直接学習するのではなく、戦略を学習する。

メソッド:
* dd4: distance-and-direction 数字とビアの位置と方向からどの4方向のビアを使えばよいか推定する

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
import nnmodel


#### CONFIGURATION ####
DIR_PROB = './problem'
DIR_DATA = './answer'
DIR_DUMP = './dump'

parser = argparse.ArgumentParser(description='train_3d -- 2016')
parser.add_argument('--size', '-s', default=3, type=int,
                    help='Window size or dimension (default: 3)')
parser.add_argument('--unit', '-u', default=100, type=int,
                    help='Number of units in hidden layer (default: 100)')
parser.add_argument('--epoch', '-e', default=1000, type=int,
                    help='Number of epochs (default: 1000)')
parser.add_argument('--test', '-t', default='none', type=str,
                    help='Team-Problem name for test (e.g. T01_A06) (default: none)')
parser.add_argument('--method', '-m', default='dd4', type=str,
                    help='Method of selecting input data type to ML: dd4 (dafault), dd8, ddx8, ddv8')
args = parser.parse_args()
print args

n_dims             = args.size
n_dims_half        = n_dims / 2
n_units            = args.unit
n_epoch            = args.epoch
testfilename       = args.test
testfilename_short = testfilename.replace('_', '')
#if '.txt' in testfilename:
#    testfilename_short = testfilename[0:-4]


# [3.1] 準備
# Prepare method --> nl.py


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 n_units 次元)
# method = dd4 の場合
#   入力: (N x N - 1) + 2 = N^2 + 1 次元
#   出力: 4次元
# method = dd8 の場合
#   入力: (N x N - 1) + 2 = N^2 + 1 次元
#   出力: 8次元
# method = ddx8 の場合
#   入力: (N x N - 1) + 2 + 2 = N^2 + 3 次元
#   出力: 8次元
# method = ddv8 の場合
#   入力: (N x N - 1) * 2 + 2 + 2 = 2*N^2 + 2 次元
#   出力: 8次元
if args.method == 'dd4':
    input_dims = n_dims**2 + 1
    output_dims = 4
elif args.method == 'dd8':
    input_dims = n_dims**2 + 1
    output_dims = 8
elif args.method == 'ddx8':
    input_dims = n_dims**2 + 3
    output_dims = 8
elif args.method == 'ddv8':
    input_dims = 2*(n_dims**2) + 2
    output_dims = 8
else:
    raise NotImplementedError()

# model = FunctionSet(l1=F.Linear(input_dims, n_units),
#                     l2=F.Linear(n_units, n_units),
#                     l3=F.Linear(n_units, output_dims))

# # Neural net architecture
# # ニューラルネットの構造
# def forward(x_data, y_data, train=True):
#     x, t = Variable(x_data), Variable(y_data)
#     h1 = F.dropout(F.relu(model.l1(x)),  train=train)
#     h2 = F.dropout(F.relu(model.l2(h1)), train=train)
#     y  = model.l3(h2)
#     # 多クラス分類なので誤差関数としてソフトマックス関数の
#     # 交差エントロピー関数を用いて、誤差を導出
#     return F.softmax_cross_entropy(y, t), F.accuracy(y, t), y

nn = nnmodel.Model1((input_dims, n_units, output_dims))
model = nn()

# [3.3] Optimizerの設定
# Setup optimizer
optimizer = optimizers.Adam()
optimizer.setup(model.collect_parameters())


# Learning loop
x_train_raw, y_train_raw = [], []
x_test_raw,  y_test_raw  = [], []

# トレーニング/テスト ファイルを読み込む
# テストファイルは1回だけ読み込む
datafiles = glob.glob(DIR_DATA + '/*.txt')
has_read = False
for datafile in datafiles:
    path_datafile = datafile.replace('\\', '/').split('/')
    datafilename = path_datafile[-1]
    datafilename_woext = datafilename[0:-4]

    # テストとして指定した問題番号は合っているけど、チーム名が違う場合 --> スキップ
    if datafilename_woext[5:] == testfilename[5:] and datafilename_woext != testfilename:
        continue

    # Training data
    if datafilename_woext != testfilename:
        sys.stdout.write('Reading train file: {}/{} ...'.format(DIR_DATA, datafilename))

        board_x, board_y, board_z, boards = nl3d.read_ansfile(datafile, n_dims)
        sys.stdout.write('  --> {} X {} X {}\n'.format(board_x, board_y, board_z))

        # 対応する問題ファイルを読み込む (ビアは問題からのみわかるため)
        probfilename = '{}/NL_Q{}'.format(DIR_PROB, datafilename[5:])
        _, _, _, pboards = nl3d.read_probfile(probfilename, n_dims)
        # 答えファイルに基づいた内部データにビア情報を付加する
        for z in range(board_z):
            for y in range(n_dims_half, board_y + n_dims_half):
                for x in range(n_dims_half, board_x + n_dims_half):
                    if pboards[z][y][x]['type'] == 'via':
                        assert(boards[z][y][x]['type'] == 1)
                        boards[z][y][x]['type'] = 'via'

        for z in range(board_z):
            x_data, y_data = nl3d.gen_dataset_dd(board_x, board_y, board_z, boards, z, n_dims, args.method)

            x_train_raw = x_train_raw + x_data
            y_train_raw = y_train_raw + y_data
    # Test data
    elif not has_read:
        sys.stdout.write('Reading test file: {}/{} ...'.format(DIR_DATA, datafilename))
        has_read = True

        board_x, board_y, board_z, boards = nl3d.read_ansfile(datafile, n_dims)
        sys.stdout.write('  ==> {} X {} X {}\n'.format(board_x, board_y, board_z))

        # 対応する問題ファイルを読み込む (ビアは問題からのみわかるため)
        probfilename = '{}/NL_Q{}'.format(DIR_PROB, datafilename[5:])
        _, _, _, pboards = nl3d.read_probfile(probfilename, n_dims)
        # 答えファイルに基づいた内部データにビア情報を付加する
        for z in range(board_z):
            for y in range(n_dims_half, board_y + n_dims_half):
                for x in range(n_dims_half, board_x + n_dims_half):
                    if pboards[z][y][x]['type'] == 'via':
                        assert(boards[z][y][x]['type'] == 1)
                        boards[z][y][x]['type'] = 'via'

        for z in range(board_z):
            x_data, y_data = nl3d.gen_dataset_dd(board_x, board_y, board_z, boards, z, n_dims, args.method)

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
        loss_train, accuracy_train, _ = nn.forward(x_batch, y_batch)

        # 誤差逆伝播で勾配を計算
        loss_train.backward()
        optimizer.update()

        sum_loss     = sum_loss + loss_train.data * batchsize
        sum_accuracy = sum_accuracy + accuracy_train.data * batchsize

    # Evaluation
    if testfilename != 'none':
        loss_test, accuracy_test, result = nn.forward(x_test, y_test, train=False)

    # 訓練データ/テストデータの誤差と、正解精度を表示
    if epoch % 10 == 0:
        print 'epoch', epoch
        print 'Train: mean loss={}, accuracy={}'.format(sum_loss / n_train, sum_accuracy / n_train)
        if testfilename != 'none':
            print 'Test:  mean loss={}, accuracy={}'.format(loss_test.data,  accuracy_test.data)

# モデルをシリアライズ化して保存
with open(DIR_DUMP + '/s{}_u{}_e{}_m{}_t{}.pkl'.format(n_dims, n_units, n_epoch, args.method, testfilename_short), 'w') as f:
    pickle.dump(model, f)
