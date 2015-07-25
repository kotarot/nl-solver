#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「テスト用スクリプト」
周囲 N x N = N^2 マスを切り出して (window) 学習してみる

参考文献:
* Python - 【機械学習】ディープラーニング フレームワークChainerを試しながら解説してみる。 - Qiita http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412
"""

import argparse
import pickle
import sys

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F

import nl


#### CONFIGURATION ####
parser = argparse.ArgumentParser(description='Machine learning based nl-solver test: WINDOW (testing)')
parser.add_argument('input', nargs=None, default=None, type=str, help='Path to input problem file')
parser.add_argument('--pickle', '-p', default=None, type=str, help='Path to input pickle (dump) model file')
args = parser.parse_args()

input_problem = args.input
input_pickle  = args.pickle

# pickle ファイル名から dims を読み取る
n_dims = -1
for token in input_pickle.split('_'):
    if 'dim' in token:
        n_dims = int(token[3:])
assert(1 <= n_dims)


# [3.1] 準備
# Prepare dataset --> nl.py


# [3.2] モデルの定義
# Prepare multi-layer perceptron model
# pickle ファイルから読み込む
with open(input_pickle, 'r') as f:
    model = pickle.load(f)


# Neural net architecture
# ニューラルネットの構造
def evaluate(x_data):
    x = Variable(x_data)
    h1 = F.dropout(F.relu(model.l1(x)),  train=False)
    h2 = F.dropout(F.relu(model.l2(h1)), train=False)
    y  = model.l3(h2)
    return y


# Testing phase
board_x, board_y, board = nl.read_probfile(input_problem, n_dims)
x_data, _ = nl.gen_dataset_shape(board_x, board_y, board, n_dims)

x_test = np.array(x_data, dtype=np.float32)

result = evaluate(x_test)

# テストデータの配線を表示
idx = 0
str = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
for y in range(n_dims / 2, board_y + n_dims / 2):
    for x in range(n_dims / 2, board_x + n_dims / 2):
        if board[y][x]['type'] == 1:
            sys.stdout.write('\033[1;30;47m ' + nl.int2str(board[y][x]['data'], 36) + ' \033[0m')
        else:
            ex_shape = np.argmax(result.data[idx])
            sys.stdout.write('\033[1;30;47m' + str[ex_shape] + '\033[0m')
            idx = idx + 1
    print ''
