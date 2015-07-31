#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「テスト用スクリプト」
周囲 N x N = N^2 マスを切り出して (window) 学習してみる

参考文献:
* Python - 【機械学習】ディープラーニング フレームワークChainerを試しながら解説してみる。 - Qiita http://qiita.com/kenmatsu4/items/7b8d24d4c5144a686412
"""

import argparse
import copy
import pickle
import sys

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F

import nl


parser = argparse.ArgumentParser(description='Machine learning based nl-solver test: WINDOW (testing)')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to input problem/answer file')
parser.add_argument('--pickle', '-p', default=None, type=str,
                    help='Path to input pickle (dump) model file')
parser.add_argument('--answer', '-a', default=False, action='store_true',
                    help='Set on to switch to answer-input mode (default: False)')
args = parser.parse_args()

input_problem = args.input
input_pickle  = args.pickle

# (1) pickle ファイル名から dims を読み取る
# (2) pickle ファイル名から dataset を読み取る
n_dims = -1
dataset = None
pickle_path = input_pickle.split('/')
pickle_filename = pickle_path[-1]
for token in pickle_filename.split('_'):
    if token[0:1] == 's':
        n_dims = int(token[1:])
    if token[0:1] == 'd':
        filetokens = token[1:].split('.')
        dataset = filetokens[0]
assert(1 <= n_dims)
assert(dataset != None)
n_dims_half = n_dims / 2


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


# レッドラインの割合を表示
# board_pr: 予想されるボード
def show_wrong_stat(board_pr):
    assert(args.answer == True)

    cells_total = board_x * board_y
    cells_true, cells_false, cells_num = 0, 0, 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if board[y][x]['type'] == 1:
                cells_true = cells_true + 1
                cells_num = cells_num + 1
            else:
                # 正しい配線形状
                if board[y][x]['shape'] == board_pr[y][x]['shape']:
                    cells_true = cells_true + 1
                # 間違ってる配線形状
                else:
                    cells_false = cells_false + 1
    print 'Total cells: {}'.format(cells_total)
    print '  success:   {} (include {} number cells)'.format(cells_true, cells_num)
    print '  failure:   {}'.format(cells_false)
    print '  rate:      {}'.format(float(cells_false) / cells_total)


# Testing phase
if (not args.answer):
    board_x, board_y, board = nl.read_probfile(input_problem, n_dims)
else:
    board_x, board_y, board = nl.read_ansfile(input_problem, n_dims)
x_data, _ = nl.gen_dataset_shape(board_x, board_y, board, n_dims, dataset)

x_test = np.array(x_data, dtype=np.float32)

result = evaluate(x_test)

# テストデータの配線を表示
idx = 0
board_pr = copy.deepcopy(board)
shstr = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
for y in range(n_dims_half, board_y + n_dims_half):
    for x in range(n_dims_half, board_x + n_dims_half):
        if board[y][x]['type'] == 1:
            sys.stdout.write('\033[1;30;47m ' + nl.int2str(board[y][x]['data'], 36) + ' \033[0m')
        else:
            ex_shape = np.argmax(result.data[idx])
            board_pr[y][x]['shape'] = ex_shape
            # 正しい配線形状
            if (not args.answer) or board[y][x]['shape'] == ex_shape:
                sys.stdout.write('\033[1;30;47m' + shstr[ex_shape] + '\033[0m')
            # 間違ってる配線形状
            else:
                sys.stdout.write('\033[1;31;47m' + shstr[ex_shape] + '\033[0m')
            idx = idx + 1
    print ''

# 答えデータを入力した場合、レッドラインの割合を表示
if args.answer:
    show_wrong_stat(board_pr)
