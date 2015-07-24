#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
周囲 N x N = N^2 マスを切り出して (window)
学習してみるテスト

参考文献:
* Python - 【機械学習】ディープラーニング フレームワークChainerを試しながら解説してみる。 - Qiita http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412
"""

import argparse
import sys
import utils

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F


#### CONFIGURATION ####
parser = argparse.ArgumentParser(description='Machine learning test: WINDOW')
parser.add_argument('--dim', '-d', default=3, type=int, help='Window dimension')
parser.add_argument('--epoch', '-e', default=100000, type=int, help='Number of epoches')
parser.add_argument('--unit', '-u', default=100, type=int, help='Number of units in hidden layer')
args = parser.parse_args()

n_dims  = args.dim
n_epoch = args.epoch
n_units = args.unit


# [3.1] 準備
# Prepare dataset
# Answerファイルを読み込む
def read_ansfile(filename):
    # board は 上下左右に番兵を (N / 2) 行
    # 各セルは data, type, dir 属性をもつ
    # * data は Ansファイル内に書いてある数字そのもの
    # * type は 0: blank, 1: 数字 (ターミナル), 2: 線  -- セル外と接続してる線数と等価
    # * shape は 配線の形 0:none 1:│ 2:┘ 3:└ 4:┐ 5:┌ 6:─
    # * dirb は 上下左右の配列 接続する場合 True、そうでない場合 False
    # * diri は 上下左右の配列 接続方向が 1 または -1 -- 1 は ソースからシンクへの経路、-1 は逆の経路
    #     ソースとシンクの定義: より左上にある方がソース
    board = []
    board_x, board_y = -1, -1 # ボードの X と Y
    #maxn_line = -1            # 線番号 (ラインナンバ) の最大値

    _board = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
        # 1行目以外
        else:
            line_x = [{'data': -1} for i in range(0, n_dims / 2)] + [{'data': int(token)} for token in line.split(',')] + [{'data': -1} for i in range(0, n_dims / 2)]
            _board.append(line_x)
    board = [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims / 2)] \
          + _board \
          + [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims / 2)]

    # 属性を記録する
    for y in range(n_dims / 2, board_y + n_dims / 2):
        for x in range(n_dims / 2, board_x + n_dims / 2):
            if board[y][x]['data'] != -1:
                # ラインナンバの最大値を更新
                #if maxn_line < board[y][x]['data']:
                #    maxn_line = board[y][x]['data']

                # type, shape, and dirb
                n_connect, s_connect = 0, 0
                d_connect = [False, False, False, False] # 上下左右
                connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                for i, c in enumerate(connects):
                    if board[y][x]['data'] == board[c[1]][c[0]]['data']:
                        n_connect = n_connect + 1
                        d_connect[i] = True
                board[y][x]['type']  = n_connect
                board[y][x]['dirb']  = d_connect
                if n_connect != 1:
                    if d_connect[0] == True and d_connect[1] == True:
                        board[y][x]['shape'] = 1
                    elif d_connect[0] == True and d_connect[2] == True:
                        board[y][x]['shape'] = 2
                    elif d_connect[0] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 3
                    elif d_connect[1] == True and d_connect[2] == True:
                        board[y][x]['shape'] = 4
                    elif d_connect[1] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 5
                    elif d_connect[2] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 6
                    else:
                        board[y][x]['shape'] = 0

    # TODO: 経路の方向の違いによる diri の更新

    #print board
    return board_x, board_y, board

# データセットを生成 (配線形状の分類)
# 入力はターミナル数字が存在するセルを 1、存在しないセルを 0、ボード外を -1 とした (N^2 - 1) 次元のベクトル
# 出力は配線形状を表す分類スカラー数字
def gen_dataset_shape(board_x, board_y, board):
    x_data, y_data = [], []
    for y in range(n_dims / 2, board_y + n_dims / 2):
        for x in range(n_dims / 2, board_x + n_dims / 2):
            if board[y][x]['type'] != 1:
                dx = []
                # 入力: window
                for wy in range(-(n_dims / 2), n_dims / 2 + 1):
                    for wx in range(-(n_dims / 2), n_dims / 2 + 1):
                        if not (wx == 0 and wy == 0):
                            if board[y + wy][x + wx]['data'] == -1:
                                dx.append(-1)
                            elif board[y + wy][x + wx]['type'] == 1:
                                dx.append(1)
                            else:
                                dx.append(0)
                x_data.append(dx)
                # 出力: direction
                y_data.append(board[y][x]['shape'])

    return x_data, y_data


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 n_units 次元)
# 入力: N x N - 1 = N^2 - 1 次元
# 出力: 7次元
model = FunctionSet(l1=F.Linear(n_dims**2 - 1, n_units),
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
train_files = ['T99_A01.txt', 'T99_A02.txt', 'T99_A03.txt', 'T99_A04.txt', 'T99_A06.txt',
               'T99_A07.txt', 'T99_A08.txt', 'T99_A09.txt', 'T99_A13.txt',
               'a01.txt', 'a02.txt', 'a03.txt', 'a04.txt', 'a05.txt',
               'a06.txt', 'a07.txt', 'a08.txt', 'a09.txt', 'a10.txt',
               'a11.txt', 'a12.txt', 'a13.txt', 'a14.txt', 'a15.txt']
test_files  = ['T99_A15.txt']

for train_file in train_files:
    print 'Reading training file: {} ...'.format(train_file)
    board_x, board_y, board = read_ansfile(train_file)

    x_data, y_data = gen_dataset_shape(board_x, board_y, board) # 配線形状の分類
    #x_data, y_data = gen_dataset_dirsrc(board_x, board_y, board) # 配線接続位置の分類 (ソースから)
    #x_data, y_data = gen_dataset_dirsnk(board_x, board_y, board) # 配線接続位置の分類 (シンクから)

    _x_train = _x_train + x_data
    _y_train = _y_train + y_data

for test_file in test_files:
    print 'Reading testing file: {} ...'.format(test_file)
    board_x, board_y, board = read_ansfile(test_file)

    x_data, y_data = gen_dataset_shape(board_x, board_y, board) # 配線形状の分類
    #x_data, y_data = gen_dataset_dirsrc(board_x, board_y, board) # 配線接続位置の分類 (ソースから)
    #x_data, y_data = gen_dataset_dirsnk(board_x, board_y, board) # 配線接続位置の分類 (シンクから)

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
        for y in range(n_dims / 2, board_y + n_dims / 2):
            for x in range(n_dims / 2, board_x + n_dims / 2):
                if board[y][x]['type'] == 1:
                    sys.stdout.write('\033[1;30;47m ' + utils.int2str(board[y][x]['data'], 36) + ' \033[0m')
                else:
                    ex_shape = np.argmax(result.data[idx])
                    # 正しい配線形状
                    if board[y][x]['shape'] == ex_shape:
                        sys.stdout.write('\033[1;30;47m' + str[ex_shape] + '\033[0m')
                    # 間違ってる配線形状
                    else:
                        sys.stdout.write('\033[1;31;47m' + str[ex_shape] + '\033[0m')
                    idx = idx + 1
            print ''
