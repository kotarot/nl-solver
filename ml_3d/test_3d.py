#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「テスト用スクリプト 3D 2016年バージョン」
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
args = parser.parse_args()
print args

input_problem = args.input
input_pickle  = args.pickle
#output_fix    = args.output

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
for z in range(board_z):
    x_data[z], _ = nl3d.gen_dataset_dd(board_x, board_y, board_z, boards, z, n_dims, method, testmode=True)

x_test = [[] for z in range(board_z)]
for z in range(board_z):
    x_test[z] = np.array(x_data[z], dtype=np.float32)

result = [[] for z in range(board_z)]
for z in range(board_z):
    result[z] = evaluate(x_test[z])

# 結果を見る (超簡易的バージョン)
for z in range(board_z):
    print 'LAYER {}'.format(z + 1)
    for d in result[z].data:
        print num_to_dd(np.argmax(d))
