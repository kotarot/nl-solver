#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
周囲 3x3 = 9 マスを切り出して (window)
学習してみるテスト

参考文献:
* Python - 【機械学習】ディープラーニング フレームワークChainerを試しながら解説してみる。 - Qiita http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412
"""

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F


# [3.1] 準備
# Prepare dataset
# Answerファイルを読み込む
def read_ansfile(filename):
    # board は 上下左右に番兵を1行
    # 各セルは data, type, dir 属性をもつ
    # * data は Ansファイル内に書いてある数字そのもの
    # * type は 0: blank, 1: 数字 (ターミナル), 2: 線  -- セル外と接続してる線数と等価
    # * shape は 配線の形 0:none 1:上 2:下 4:左 8:右 3:│ 12:─ 9:└ 10:┌ 6:┐ 5:┘ -- 上下左右をビットと考える
    #     --> 7が空いてるので12を7にして 0~10 にマッピング
    # * dirb は 上下左右の配列 接続する場合 True、そうでない場合 False
    # * diri は 上下左右の配列 接続方向が 1 または -1 -- 1 は ソースからシンクへの経路、-1 は逆の経路
    #     ソースとシンクの定義: より左上にある方がソース
    board = []
    board_x, board_y = -1, -1 # ボードの X と Y
    maxn_line = -1            # 線番号 (ラインナンバ) の最大値

    _board = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
        # 1行目以外
        else:
            line_x = [{'data': -1}] + [{'data': int(token)} for token in line.split(',')] + [{'data': -1}]
            _board.append(line_x)
    board = [[{'data': -1} for i in range(0, board_x + 2)]] + _board + [[{'data': -1} for i in range(0, board_x + 2)]]

    # 属性を記録する
    for y in range(1, board_y + 1):
        for x in range(1, board_x + 1):
            if board[y][x]['data'] != -1:
                # ラインナンバの最大値を更新
                if maxn_line < board[y][x]['data']:
                    maxn_line = board[y][x]['data']

                # type, shape, and dirb
                n_connect, s_connect = 0, 0
                d_connect = [False, False, False, False] # 上下左右
                connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                for i, c in enumerate(connects):
                    if board[y][x]['data'] == board[c[1]][c[0]]['data']:
                        n_connect = n_connect + 1
                        s_connect = s_connect + 2**i
                        d_connect[i] = True
                board[y][x]['type']  = n_connect
                #board[y][x]['shape'] = s_connect
                board[y][x]['dirb']  = d_connect
                if s_connect == 12:
                    board[y][x]['shape'] = 7
                else:
                    board[y][x]['shape'] = s_connect

    # TODO: 経路の方向の違いによる diri の更新

    #print board
    return board_x, board_y, board

# データセットを生成 (配線形状の分類)
# 入力はターミナル数字が存在するセルを 1、存在しないセルを 0、ボード外を -1 とした 9次元のベクトル
# 出力は配線形状を表す分類スカラー数字
def gen_dataset_shape(board_x, board_y, board):
    x_data, y_data = [], []
    for y in range(0, board_y):
        for x in range(0, board_x):
            dx = []
            # 入力: window
            for wy in range(0, 3):
                for wx in range(0, 3):
                    if board[y + wy][x + wx]['data'] == -1:
                        dx.append(-1)
                    elif board[y + wy][x + wx]['type'] == 1:
                        dx.append(1)
                    else:
                        dx.append(0)
            x_data.append(dx)
            # 出力: direction
            y_data.append(board[y + 1][x + 1]['shape'])

    #return np.array(x_data, dtype=np.float32), np.array(y_data, dtype=np.int32)
    return x_data, y_data


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 100次元)
# 入力: 3x3 = 9次元
# 出力: 11次元
model = FunctionSet(l1=F.Linear(9, 100),
                    l2=F.Linear(100, 100),
                    l3=F.Linear(100, 11))

# Neural net architecture
# ニューラルネットの構造
def forward(x_data, y_data, train=True):
    x, t = Variable(x_data), Variable(y_data)
    h1 = F.dropout(F.relu(model.l1(x)),  train=train)
    h2 = F.dropout(F.relu(model.l2(h1)), train=train)
    y  = model.l3(h2)
    # 多クラス分類なので誤差関数としてソフトマックス関数の
    # 交差エントロピー関数を用いて、誤差を導出
    return F.softmax_cross_entropy(y, t), F.accuracy(y, t)


# [3.3] Optimizerの設定
# Setup optimizer
optimizer = optimizers.Adam()
optimizer.setup(model.collect_parameters())


# Learning loop
_x_train, _y_train = [], []

# 複数ファイルを読み込む
training_files = ['T99_A01.txt', 'T99_A02.txt', 'T99_A03.txt', 'T99_A04.txt', 'T99_A06.txt',
                  'T99_A07.txt', 'T99_A08.txt', 'T99_A09.txt', 'T99_A13.txt']

for file in training_files:
    print 'Reading {} ...'.format(file)
    board_x, board_y, board = read_ansfile(file)

    x_data, y_data = gen_dataset_shape(board_x, board_y, board) # 配線形状の分類
    #x_data, y_data = gen_dataset_dirsrc(board_x, board_y, board) # 配線接続位置の分類 (ソースから)
    #x_data, y_data = gen_dataset_dirsnk(board_x, board_y, board) # 配線接続位置の分類 (シンクから)

    _x_train = _x_train + x_data
    _y_train = _y_train + y_data

x_train = np.array(_x_train, dtype=np.float32)
y_train = np.array(_y_train, dtype=np.int32)
#print x_train
#print y_train

for epoch in xrange(1, 10000 + 1):
    # Training
    # バッチサイズごとに学習する方法もある
    # http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412

    # 勾配を初期化
    optimizer.zero_grads()

    # 順伝播させて誤差と精度を算出
    loss, accuracy = forward(x_train, y_train)

    # 誤差逆伝播で勾配を計算
    loss.backward()
    optimizer.update()

    # 訓練データの誤差と、正解精度を表示
    if epoch % 100 == 0:
        print 'epoch', epoch
        print 'Training: mean loss={}, accuracy={}'.format(loss.data, accuracy.data)
