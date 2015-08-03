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
    print '  success:   {} (include {} number-cells)'.format(cells_true, cells_num)
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
            assert(ex_shape != 0)
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

# 線が途切れてるセルを特定する
for y in range(n_dims_half, board_y + n_dims_half):
    for x in range(n_dims_half, board_x + n_dims_half):
        # 上下左右で途切れている線
        gap_arround = {'u': False, 'd': False, 'l': False, 'r': False}
        if board_pr[y][x]['type'] != 1:
            # 中心セルの形状
            shape_center = board_pr[y][x]['shape']
            # 周囲セル (上下左右) のタイプと形状
            type_arround = {'u': board_pr[y - 1][x]['type'], 'd': board_pr[y + 1][x]['type'],
                            'l': board_pr[y][x - 1]['type'], 'r': board_pr[y][x + 1]['type']}
            shape_arround = {'u': board_pr[y - 1][x]['shape'], 'd': board_pr[y + 1][x]['shape'],
                             'l': board_pr[y][x - 1]['shape'], 'r': board_pr[y][x + 1]['shape']}

            # 上方向
            if shape_center == 1 or shape_center == 2 or shape_center == 3:
                if type_arround['u'] != 1 and shape_arround['u'] != 1 and shape_arround['u'] != 4 and shape_arround['u'] != 5:
                    gap_arround['u'] = True
            # 下方向
            if shape_center == 1 or shape_center == 4 or shape_center == 5:
                if type_arround['d'] != 1 and shape_arround['d'] != 1 and shape_arround['d'] != 2 and shape_arround['d'] != 3:
                    gap_arround['d'] = True
            # 左方向
            if shape_center == 2 or shape_center == 4 or shape_center == 6:
                if type_arround['l'] != 1 and shape_arround['l'] != 3 and shape_arround['l'] != 5 and shape_arround['l'] != 6:
                    gap_arround['l'] = True
            # 右方向
            if shape_center == 3 or shape_center == 5 or shape_center == 6:
                if type_arround['r'] != 1 and shape_arround['r'] != 2 and shape_arround['r'] != 4 and shape_arround['r'] != 6:
                    gap_arround['r'] = True

        # 途切れているセル
        if gap_arround['u'] or gap_arround['d'] or gap_arround['l'] or gap_arround['r']:
            board_pr[y][x]['hasgap'] = True
        else:
            board_pr[y][x]['hasgap'] = False

# 確認
#for y in range(n_dims_half, board_y + n_dims_half):
#    for x in range(n_dims_half, board_x + n_dims_half):
#        if board[y][x]['hasgap']:
#            print '({}, {}) has a gap.'.format(x - n_dims_half, y - n_dims_half)
idx = 0
for y in range(n_dims_half, board_y + n_dims_half):
    for x in range(n_dims_half, board_x + n_dims_half):
        if board[y][x]['type'] == 1:
            sys.stdout.write('\033[1;30;47m ' + nl.int2str(board[y][x]['data'], 36) + ' \033[0m')
        else:
            # 正しい配線形状 / 間違ってる配線形状
            if board[y][x]['shape'] == board_pr[y][x]['shape']:
                fr_color = '30'
            else:
                fr_color = '31'

            # 途切れてないセル / 途切れてるセル
            if board_pr[y][x]['hasgap'] == False:
                bg_color = '47'
            else:
                bg_color = '43'

            sys.stdout.write('\033[1;{};{}m{}\033[0m'.format(fr_color, bg_color, shstr[board_pr[y][x]['shape']]))

            idx = idx + 1
    print ''

# レッドラインカバー率を計算
#cells_total = board_x * board_y
cells_red, cells_falseneg = 0, 0
for y in range(n_dims_half, board_y + n_dims_half):
    for x in range(n_dims_half, board_x + n_dims_half):
        if board[y][x]['type'] != 1:
            # 間違っている配線 (レッドライン)
            if board[y][x]['shape'] != board_pr[y][x]['shape']:
                cells_red = cells_red + 1
            # 間違っている配線 (レッドライン) かつ引き剥がすセルに指定されていない
            if board[y][x]['shape'] != board_pr[y][x]['shape'] and board_pr[y][x]['hasgap']:
                cells_falseneg = cells_falseneg + 1
print 'Coverage rate: {}% ({} / {})'.format(cells_falseneg * 100.0 / cells_red, cells_falseneg, cells_red)
