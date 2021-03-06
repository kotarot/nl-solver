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
import nl3d


parser = argparse.ArgumentParser(description='Machine learning based nl-solver test: WINDOW (testing)')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to input problem/answer file')
parser.add_argument('--pickle', '-p', default=None, type=str,
                    help='Path to input pickle (dump) model file')
parser.add_argument('--level', '-l', default=2, type=int,
                    help='Limit level of coverage expansion (default: 2)')
parser.add_argument('--answer', '-a', default=False, action='store_true',
                    help='Set on to switch to answer-input mode (default: False)')
parser.add_argument('--output', '-o', default=None, type=str,
                    help='Path to output fix-file')
args = parser.parse_args()
print args

input_problem = args.input
input_pickle  = args.pickle
output_fix    = args.output

# (1) pickle ファイル名から dims を読み取る
# (2) pickle ファイル名から method を読み取る
n_dims = -1
method = None
pickle_path = input_pickle.split('/')
pickle_filename = pickle_path[-1]
for token in pickle_filename.split('_'):
    if token[0:1] == 's':
        n_dims = int(token[1:])
    if token[0:1] == 'm':
        filetokens = token[1:].split('.')
        method = filetokens[0]
assert(1 <= n_dims)
assert(method != None)
n_dims_half = n_dims / 2


# [3.1] 準備
# Prepare method --> nl.py


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


# 配線を表示する
def show_board(_board, show_float=False):
    shstr = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
    idx = 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if _board[y][x]['type'] == 1:
                sys.stdout.write('\033[1;30;47m {} \033[0m'.format(nl.int2str(_board[y][x]['data'], 36)))
            else:
                # 正しい配線形状 / 間違ってる配線形状
                fr_color = '30'
                if args.answer and _board[y][x]['wrong']:
                    fr_color = '31'

                # 途切れてないセル / 途切れてるセル
                bg_color = '47'
                if show_float:
                    if _board[y][x]['float'] != None:
                        bg_color = '43'

                sys.stdout.write('\033[1;{};{}m{}\033[0m'.format(fr_color, bg_color, shstr[_board[y][x]['shape']]))

                idx = idx + 1
        print ''


# 線が途切れてるセルを特定する
# `hascap` に真偽を記録する
def find_gapcells(board_pr):
    _board = copy.deepcopy(board_pr)
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            # 上下左右で途切れている線
            gap_arround = {'u': False, 'd': False, 'l': False, 'r': False}
            if _board[y][x]['type'] != 1:
                # 中心セルの形状
                shape_center = _board[y][x]['shape']
                # 周囲セル (上下左右) のタイプと形状
                type_arround = {'u': _board[y - 1][x]['type'], 'd': _board[y + 1][x]['type'],
                                'l': _board[y][x - 1]['type'], 'r': _board[y][x + 1]['type']}
                shape_arround = {'u': _board[y - 1][x]['shape'], 'd': _board[y + 1][x]['shape'],
                                 'l': _board[y][x - 1]['shape'], 'r': _board[y][x + 1]['shape']}

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

            # 途切れているセルを記録
            if gap_arround['u'] or gap_arround['d'] or gap_arround['l'] or gap_arround['r']:
                _board[y][x]['hasgap'] = True
            else:
                _board[y][x]['hasgap'] = False

    return _board


# レッドラインの割合を計算/表示
# board_pr: 予想されるボード
def show_wrong_stat(board_pr):
    assert(args.answer == True)

    cells_total = board_x * board_y
    cells_true, cells_false, cells_num = 0, 0, 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if board_pr[y][x]['type'] == 1:
                cells_true = cells_true + 1
                cells_num = cells_num + 1
            else:
                # 正しい配線形状
                if not board_pr[y][x]['wrong']:
                    cells_true = cells_true + 1
                # 間違ってる配線形状
                else:
                    cells_false = cells_false + 1
    print 'Total cells: {}'.format(cells_total)
    print '  success:   {} (include {} number-cells)'.format(cells_true, cells_num)
    print '  failure:   {}'.format(cells_false)
    print '  rate:      {}'.format(float(cells_false) / cells_total)


# レッドラインカバー率を計算/表示
def show_coveragerate(board_pr):
    cells_red, cells_falseneg = 0, 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if board_pr[y][x]['type'] != 1:
                # 間違っている配線 (レッドライン)
                if board_pr[y][x]['wrong']:
                    cells_red = cells_red + 1
                # 間違っている配線 (レッドライン) かつ引き剥がすセルに指定されていない
                if board_pr[y][x]['wrong'] and board_pr[y][x]['float'] is not None:
                    cells_falseneg = cells_falseneg + 1
    print 'Coverage rate: {}% ({} / {})'.format(cells_falseneg * 100.0 / cells_red, cells_falseneg, cells_red)


# ボードと配線の座標を入力したとき、
# その座標を含む配線の両端の数字セルの数字をタプルで返す
# Issue #78: 配線ループ時は適当回数で打ち切る
def find_terminals(_board, x, y):
    assert(_board[y][x]['type'] != 1)

    if _board[y][x]['shape'] == 1:
        return (_find_terminals(_board, x, y - 1, 'south', 0), _find_terminals(_board, x, y + 1, 'north', 0))
    elif _board[y][x]['shape'] == 2:
        return (_find_terminals(_board, x, y - 1, 'south', 0), _find_terminals(_board, x - 1, y, 'east', 0))
    elif _board[y][x]['shape'] == 3:
        return (_find_terminals(_board, x, y - 1, 'south', 0), _find_terminals(_board, x + 1, y, 'west', 0))
    elif _board[y][x]['shape'] == 4:
        return (_find_terminals(_board, x, y + 1, 'north', 0), _find_terminals(_board, x - 1, y, 'east', 0))
    elif _board[y][x]['shape'] == 5:
        return (_find_terminals(_board, x, y + 1, 'north', 0), _find_terminals(_board, x + 1, y, 'west', 0))
    elif _board[y][x]['shape'] == 6:
        return (_find_terminals(_board, x - 1, y, 'east', 0), _find_terminals(_board, x + 1, y, 'west', 0))
    else:
        raise Exception('Never reachable here')

def _find_terminals(_board, x, y, _from, depth):
    if 256 < depth:
        return None
    depth = depth + 1

    if _board[y][x]['type'] <= 0:
        return None
    elif _board[y][x]['type'] == 1:
        return _board[y][x]['data']
    else:
        if _board[y][x]['shape'] == 0:
            return None
        elif _board[y][x]['shape'] == 1:
            if _from == 'south':
                return _find_terminals(_board, x, y - 1, 'south', depth)
            elif _from == 'north':
                return _find_terminals(_board, x, y + 1, 'north', depth)
            else:
                return None
        elif _board[y][x]['shape'] == 2:
            if _from == 'west':
                return _find_terminals(_board, x, y - 1, 'south', depth)
            elif _from == 'north':
                return _find_terminals(_board, x - 1, y, 'east', depth)
            else:
                return None
        elif _board[y][x]['shape'] == 3:
            if _from == 'east':
                return _find_terminals(_board, x, y - 1, 'south', depth)
            elif _from == 'north':
                return _find_terminals(_board, x + 1, y, 'west', depth)
            else:
                return None
        elif _board[y][x]['shape'] == 4:
            if _from == 'west':
                return _find_terminals(_board, x, y + 1, 'north', depth)
            elif _from == 'south':
                return _find_terminals(_board, x - 1, y, 'east', depth)
            else:
                return None
        elif _board[y][x]['shape'] == 5:
            if _from == 'east':
                return _find_terminals(_board, x, y + 1, 'north', depth)
            elif _from == 'south':
                return _find_terminals(_board, x + 1, y, 'west', depth)
            else:
                return None
        elif _board[y][x]['shape'] == 6:
            if _from == 'east':
                return _find_terminals(_board, x - 1, y, 'east', depth)
            elif _from == 'west':
                return _find_terminals(_board, x + 1, y, 'west', depth)
            else:
                return None
        else:
            raise Exception('Never reachable here')


# ボードと配線の座標を入力したとき、
# その座標を含む配線の座標のリスト (path) を返す
# Issue #78: 配線ループ時は適当回数で打ち切る
def find_path(_board, x, y):
    assert(_board[y][x]['type'] != 1)

    start = [{'x': x, 'y': y}]
    if _board[y][x]['shape'] == 1:
        return _find_path(_board, x, y - 1, 'south', 0) + start + _find_path(_board, x, y + 1, 'north', 0)
    elif _board[y][x]['shape'] == 2:
        return _find_path(_board, x, y - 1, 'south', 0) + start + _find_path(_board, x - 1, y, 'east', 0)
    elif _board[y][x]['shape'] == 3:
        return _find_path(_board, x, y - 1, 'south', 0) + start + _find_path(_board, x + 1, y, 'west', 0)
    elif _board[y][x]['shape'] == 4:
        return _find_path(_board, x, y + 1, 'north', 0) + start + _find_path(_board, x - 1, y, 'east', 0)
    elif _board[y][x]['shape'] == 5:
        return _find_path(_board, x, y + 1, 'north', 0) + start + _find_path(_board, x + 1, y, 'west', 0)
    elif _board[y][x]['shape'] == 6:
        return _find_path(_board, x - 1, y, 'east', 0) + start + _find_path(_board, x + 1, y, 'west', 0)
    else:
        raise Exception('Never reachable here')

def _find_path(_board, x, y, _from, depth):
    if 256 < depth:
        return []
    depth = depth + 1

    cur = [{'x': x, 'y': y}]

    if _board[y][x]['type'] == 0:
        return []
    elif _board[y][x]['type'] == 1:
        return []
    else:
        if _board[y][x]['shape'] == 0:
            return []
        elif _board[y][x]['shape'] == 1:
            if _from == 'south':
                return _find_path(_board, x, y - 1, 'south', depth) + cur
            elif _from == 'north':
                return _find_path(_board, x, y + 1, 'north', depth) + cur
            else:
                return []
        elif _board[y][x]['shape'] == 2:
            if _from == 'west':
                return _find_path(_board, x, y - 1, 'south', depth) + cur
            elif _from == 'north':
                return _find_path(_board, x - 1, y, 'east', depth) + cur
            else:
                return []
        elif _board[y][x]['shape'] == 3:
            if _from == 'east':
                return _find_path(_board, x, y - 1, 'south', depth) + cur
            elif _from == 'north':
                return _find_path(_board, x + 1, y, 'west', depth) + cur
            else:
                return []
        elif _board[y][x]['shape'] == 4:
            if _from == 'west':
                return _find_path(_board, x, y + 1, 'north', depth) + cur
            elif _from == 'south':
                return _find_path(_board, x - 1, y, 'east', depth) + cur
            else:
                return []
        elif _board[y][x]['shape'] == 5:
            if _from == 'east':
                return _find_path(_board, x, y + 1, 'north', depth) + cur
            elif _from == 'south':
                return _find_path(_board, x + 1, y, 'west', depth) + cur
            else:
                return []
        elif _board[y][x]['shape'] == 6:
            if _from == 'east':
                return _find_path(_board, x - 1, y, 'east', depth) + cur
            elif _from == 'west':
                return _find_path(_board, x + 1, y, 'west', depth) + cur
            else:
                return []
        else:
            #raise Exception('Never reachable here')
            return []


# Testing phase
if (not args.answer):
    board_x, board_y, board_z, boards = nl3d.read_probfile(input_problem, n_dims)
else:
    board_x, board_y, board_z, boards = nl3d.read_ansfile(input_problem, n_dims)

x_data = [[] for z in range(board_z)]
for z in range(board_z):
    x_data[z], _ = nl.gen_dataset_shape(board_x, board_y, boards[z], n_dims, method)

x_test = [[] for z in range(board_z)]
for z in range(board_z):
    x_test[z] = np.array(x_data[z], dtype=np.float32)

result = [[] for z in range(board_z)]
for z in range(board_z):
    result[z] = evaluate(x_test[z])

# boards_pr: 予想される配線のボード
# `wrong` に間違った配線形状かどうか記録する
boards_pr = copy.deepcopy(boards)
for z in range(board_z):
    idx = 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] != 1:
                boards_pr[z][y][x]['shape'] = np.argmax(result[z].data[idx])
                if boards[z][y][x]['type'] != boards_pr[z][y][x]['type'] or boards[z][y][x]['shape'] != boards_pr[z][y][x]['shape']:
                    boards_pr[z][y][x]['wrong'] = True
                else:
                    boards_pr[z][y][x]['wrong'] = False
                idx = idx + 1

# テストデータの配線を表示
for z in range(board_z):
    print 'LAYER {}'.format(z + 1)
    show_board(boards_pr[z])

# 答えデータを入力した場合   ...... 答えを参照してチェック
#             入力しない場合 ...... 指定したレベルの fix-file を出力
if args.answer:
    for z in range(board_z):
        show_wrong_stat(boards_pr[z])

# 浮きセルをリセット
# 浮きセルはタッチアンドクロス手法で固定しないセルのこと
# None または 文字列 が格納される
for z in range(board_z):
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            boards_pr[z][y][x]['float'] = None

# [レベル 0]
# 以下を浮きセルとする
#   (1) 途切れているセル
#   (2) 空欄セル
#   (3) 異なる数字セル同士を結んでしまっている配線全体
#   (4) 同じ数字セルから複数の配線が出ている場合その配線全体、ただし正しい数字同士を結ぶ配線は除く
#   ((5) 孤立した配線)

print ''
print '[Level 0]'

# (1) 線が途切れてるセルを特定して浮きセルに設定する
for z in range(board_z):
    boards_pr[z] = find_gapcells(boards_pr[z])
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] != 1:
                if boards_pr[z][y][x]['hasgap']:
                    boards_pr[z][y][x]['float'] = '1-gap'

# (2) 空欄
for z in range(board_z):
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] == 0 or boards_pr[z][y][x]['shape'] == 0:
                boards_pr[z][y][x]['float'] = '2-blank'

# (3) 異なる数字セルを結んでしまっている
for z in range(board_z):
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] != 1 and boards_pr[z][y][x]['float'] is None:
                terminals = find_terminals(boards_pr[z], x, y)
                if terminals[0] != None and terminals[1] != None and terminals[0] != terminals[1]:
                    path = find_path(boards_pr[z], x, y)
                    for cell in path:
                        boards_pr[z][cell['y']][cell['x']]['float'] = '3-diff'

# (4) 同じ数字から複数
for z in range(board_z):
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] == 1:
                cand = []
                # 上 (北)
                if boards_pr[z][y - 1][x]['type'] == 2 and boards_pr[z][y - 1][x]['shape'] in [1, 4, 5]:
                    cand = cand + [{'x': x, 'y': y - 1}]
                # 下 (南)
                if boards_pr[z][y + 1][x]['type'] == 2 and boards_pr[z][y + 1][x]['shape'] in [1, 2, 3]:
                    cand = cand + [{'x': x, 'y': y + 1}]
                # 左 (西)
                if boards_pr[z][y][x - 1]['type'] == 2 and boards_pr[z][y][x - 1]['shape'] in [3, 5, 6]:
                    cand = cand + [{'x': x - 1, 'y': y}]
                # 右 (東)
                if boards_pr[z][y][x + 1]['type'] == 2 and boards_pr[z][y][x + 1]['shape'] in [2, 4, 6]:
                    cand = cand + [{'x': x + 1, 'y': y}]
                for c in cand:
                    terminals = find_terminals(boards_pr[z], c['x'], c['y'])
                    if terminals[0] != None and terminals[1] != None and terminals[0] != terminals[1]:
                        path = find_path(boards_pr[z], c['x'], c['y'])
                        for cell in path:
                            boards_pr[z][cell['y']][cell['x']]['float'] = '4-mult'

# (3)' (4)' 同じ数字セルを結んでいる線は戻す
for z in range(board_z):
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if boards_pr[z][y][x]['type'] != 1 and (boards_pr[z][y][x]['float'] == '3-diff' or boards_pr[z][y][x]['float'] == '4-mult'):
                terminals = find_terminals(boards_pr[z], x, y)
                if terminals[0] != None and terminals[1] != None and terminals[0] == terminals[1]:
                    path = find_path(boards_pr[z], x, y)
                    for cell in path:
                        boards_pr[z][cell['y']][cell['x']]['float'] = None

# 配線の表示とレッドラインカバー率を計算
for z in range(board_z):
    print 'LAYER {}'.format(z + 1)
    show_board(boards_pr[z], True)
    if args.answer:
        show_coveragerate(boards_pr[z])

# [レベル n]
# レベル n - 1 で途切れセルとして浮きセルに記録されたセルを
# 上下左右に1マスずつ拡張させる
# TODO: 既に同じ数字セル同士を結ぶ配線は固定化しても良いかも？
for level in range(1, args.level + 1):

    print ''
    print '[Level {}]'.format(level)

    _boards_pr = copy.deepcopy(boards_pr)
    for z in range(board_z):
        for y in range(n_dims_half, board_y + n_dims_half):
            for x in range(n_dims_half, board_x + n_dims_half):
                if boards_pr[z][y][x]['float'] == '1-gap':
                    _boards_pr[z][y - 1][x]['float'] = '1-gap'
                    _boards_pr[z][y + 1][x]['float'] = '1-gap'
                    _boards_pr[z][y][x - 1]['float'] = '1-gap'
                    _boards_pr[z][y][x + 1]['float'] = '1-gap'
    boards_pr = copy.deepcopy(_boards_pr)

    # 配線の表示とレッドラインカバー率を計算
    for z in range(board_z):
        print 'LAYER {}'.format(z + 1)
        show_board(boards_pr[z], True)
        if args.answer:
            show_coveragerate(boards_pr[z])

# Fix ファイルに書き込む
if not args.answer:

    if output_fix is None:
        output_fix = 'fix.txt'

    with open(output_fix, 'w') as f:
        for z in range(board_z):
            for y in range(n_dims_half, board_y + n_dims_half):
                line = ''
                for x in range(n_dims_half, board_x + n_dims_half):
                    if boards_pr[z][y][x]['float'] is None:
                        line = line + str(boards_pr[z][y][x]['shape'])
                    else:
                        line = line + '0'
                f.write(line + '\n')

        print ''
        print 'Finish writing to fix-file `{}`.'.format(output_fix)
