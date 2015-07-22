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

# board は 上下左右に番兵を1行
# 各セルは data, type, dir 属性をもつ
# * data は Ansファイル内に書いてある数字そのもの
# * type は 0: blank, 1: 数字 (ターミナル), 2: 線  -- セル外と接続してる線数と等価
# * dir  は 上下左右の配列 接続方向が True
board = []
board_x, board_y = -1, -1

# Answerファイルを読み込む
def read_ansfile(filename):
    iboard = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
        # 1行目以外
        else:
            line_x = [{'data': -1}] + [{'data': int(token)} for token in line.split(',')] + [{'data': -1}]
            iboard.append(line_x)
    board = [[{'data': -1} for i in range(0, board_x + 2)]] + iboard + [[{'data': -1} for i in range(0, board_x + 2)]]

    # 属性を記録する
    for y in range(1, board_x + 1):
        for x in range(1, board_y + 1):
            if board[y][x]['data'] != -1:
                # type and dir
                connect_num = 0
                connect_dir = [False, False, False, False] # 上下左右
                connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                for i, c in enumerate(connects):
                    if board[y][x]['data'] == board[c[1]][c[0]]['data']:
                        connect_num = connect_num + 1
                        connect_dir[i] = True
                board[y][x]['type'] = connect_num
                board[y][x]['dir'] = connect_dir

    #print board_x
    #print board_y
    #print board


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 100次元)
# 入力: 3x3 = 9次元
# 出力: 4次元
model = FunctionSet(l1=F.Linear(9, 100),
                    l2=F.Linear(100, 100),
                    l3=F.Linear(100, 4))

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


read_ansfile('T99_A01.txt')

# Learning loop
#for epoch in xrange(1, 1000):
#    print 'epoch', epoch

    # Training
    
