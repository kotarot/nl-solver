#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「テスト用スクリプト 3D 2016年バージョン」
"""

import argparse
import copy
import pickle
import sys
import random

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F

import nl
import nl3d
import nnmodel
import board


parser = argparse.ArgumentParser(description='test_3d -- 2016')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to input problem/answer file')
parser.add_argument('--pickle', '-p', default=None, type=str,
                    help='Path to input pickle (dump) model file')
#parser.add_argument('--level', '-l', default=2, type=int,
#                    help='Limit level of coverage expansion (default: 2)')
#parser.add_argument('--answer', '-a', default=False, action='store_true',
#                    help='Set on to switch to answer-input mode (default: False)')
#parser.add_argument('--output', '-o', default=None, type=str,
#                    help='Path to output fix-file')
parser.add_argument('--mode', '-m', default='single', type=str,
                    help='Output mode : *single, multi (* is default)')
args = parser.parse_args()
print args

input_problem = args.input
input_pickle  = args.pickle
#output_fix    = args.output
mode = args.mode

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


# # Neural net architecture
# # ニューラルネットの構造
# def evaluate(x_data):
#     x = Variable(x_data)
#     h1 = F.dropout(F.relu(model.l1(x)),  train=False)
#     h2 = F.dropout(F.relu(model.l2(h1)), train=False)
#     y  = model.l3(h2)
#     return y


# 数字から方向・距離を表す文字列にマッピング
def num_to_dd(n):
    if n == 0:
        return 'ne (near)'
    elif n == 1:
        return 'ne (far)'
    elif n == 2:
        return 'nw (near)'
    elif n == 3:
        return 'nw (far)'
    elif n == 4:
        return 'sw (near)'
    elif n == 5:
        return 'sw (far)'
    elif n == 6:
        return 'se (near)'
    elif n == 7:
        return 'se (far)'


# Testing phase
#if (not args.answer):
board_x, board_y, board_z, boards = nl3d.read_probfile(input_problem, n_dims)
#else:
#    board_x, board_y, board_z, boards = nl3d.read_ansfile(input_problem, n_dims)

x_data = [[] for z in range(board_z)]
names = [[] for z in range(board_z)]
for z in range(board_z):
    x_data[z], names[z] = nl3d.gen_dataset_dd(board_x, board_y, board_z, boards, z, n_dims, method, testmode=True)

x_test = [[] for z in range(board_z)]
for z in range(board_z):
    x_test[z] = np.array(x_data[z], dtype=np.float32)

nn = nnmodel.Model1(model)

result = [[] for z in range(board_z)]
for z in range(board_z):
    result[z] = nn.evaluate(x_test[z])

b = board.Board(boards, n_dims)

via_candidates = []

# 結果を見る
for z in range(board_z):
    print 'LAYER {}'.format(z + 1)
    print 'Vias : {}'.format(b.search_via(z=z+1))
    for k, d in enumerate(result[z].data):

        dlist = sorted(enumerate(d), key=lambda x: x[1], reverse=True)

        # dist = np.argmax(d)
        line = b.search_line(names[z][k], z=z+1)
        # print names[z][k], num_to_dd(dist), line
        print names[z][k], num_to_dd(dlist[0][0]), line

        vias = []
        _x, _y, _z = line

        # 周囲の状況を調べる
        sorroundings = b.get_sorroundings(1, line)
        obstacles = 0
        adjacent_via = []
        for txy in [(-1,0), (1,0), (0,-1), (0,1)]:
            tx = txy[0]
            ty = txy[1]
            if sorroundings[ty][tx] == -1.0 or sorroundings[ty][tx] == 0.5:
                obstacles += 1
            elif sorroundings[ty][tx] == 1.0:
                adjacent_via.append((tx, ty))

        # 周囲が囲まれていて，viaが隣接しているときはそのviaを使う
        if obstacles == 3 and len(adjacent_via) == 1:
            tv = adjacent_via[0]
            tvias = b.search_via(x=_x+tv[0], y=_y+tv[1], z=z+1)
            vias.append((tvias[0], 100))
        # それ以外の場合，学習結果の重みにもとづきビア候補を列挙
        else:
            for _dist in dlist:
                dist = _dist[0]
                tvias = []
                if dist == 0 or dist == 1:
                    tvias = b.search_via(x=lambda n: n>_x, y=lambda n: n<=_y, z=z+1)
                if dist == 2 or dist == 3:
                    tvias = b.search_via(x=lambda n: n<=_x, y=lambda n: n<=_y, z=z+1)
                if dist == 4 or dist == 5:
                    tvias = b.search_via(x=lambda n: n<=_x, y=lambda n: n>_y, z=z+1)
                if dist == 6 or dist == 7:
                    tvias = b.search_via(x=lambda n: n>_x, y=lambda n: n>_y, z=z+1)
                for via in tvias:
                    # 機械学習のスコアに，距離を加えてみる．
                    # 距離の逆数を加えると，距離が短いほどスコアが高くなる
                    mdist = board.Board.mdist(line, via[1])
                    if dist % 2 == 0:
                        if mdist <= n_dims:
                            vias.append((via, _dist[1]+1.0/mdist))
                    else:
                        if mdist > n_dims:
                            vias.append((via, _dist[1]+1.0/mdist))

        # print "  Via candidates:"
        for i, v in enumerate(vias):
            # print "    {}, {} ({})".format(i, v[0], v[1])
            via_candidates.append({'layer':z+1, 'line':names[z][k], 'candidates':v})

via_candidates = sorted(via_candidates, key=lambda v: v['candidates'][1], reverse=True)

vias_key = b.get_vias_key()
lines_key = b.get_lines_key()

assigned_vias_key = {}
assigend_lines = {}
confirmed_vias = {}

# ここでvia割り当て作業
for v in via_candidates:
    _key = v['candidates'][0][0]
    _layer = v['layer']
    _line = v['line']
    # print _key, v
    if not _key in assigned_vias_key:
        if _line in assigend_lines:
            pass
        else:
            assigned_vias_key[_key] = []
            assigned_vias_key[_key].append({'layer':_layer, 'line':_line, 'via':v['candidates'][0]})
            print "assign: ", _key, v 
    elif not _key in confirmed_vias:
        if not _line in assigend_lines:
            flag = True
            for v2 in assigned_vias_key[_key]:
                if v2['layer'] != _layer and v2['line'] == _line:
                    assigned_vias_key[_key].append({'layer':_layer, 'line':_line, 'via':v['candidates'][0]})
                    assigend_lines[_line] = _key
                    confirmed_vias[_key] = {'line':_line, 'via':v['candidates'][0][1], 'prob':v['candidates'][1]}
                    flag = False
                    print "assign decide: ", _key, v 
                else:
                    pass
            if flag:
                assigned_vias_key[_key].append({'layer':_layer, 'line':_line, 'via':v['candidates'][0]})
                print "assign: ", _key, v 
        else:
            pass
    else:
        pass

# 全てのviaを割り当てたか確認．割り当てられてなかったら適当に割り当てる．
not_assigned_via = []
not_assigned_line = []
for v in vias_key:
    if not v in confirmed_vias:
        not_assigned_via.append(v)
for v in lines_key:
    if not v in assigend_lines:
        not_assigned_line.append(v)

random.shuffle(not_assigned_via)

for k, v in sorted(confirmed_vias.items(), key=lambda x: x[1]['prob'], reverse=True):
    print k, v
    if mode == 'single' or v['prob']>5:
        b.set_via_to_line(k, v['line'])

if mode == 'single':
    for k, v in enumerate(not_assigned_via):
        b.set_via_to_line(v, not_assigned_line[k])

b.output_boards(mode=mode)
