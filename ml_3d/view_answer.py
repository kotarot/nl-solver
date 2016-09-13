#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「答えを表示 3Dバージョン」
"""

import argparse
import sys

import nl
import nl3d


parser = argparse.ArgumentParser(description='View answer 3D version')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to answer file')
parser.add_argument('--problem', '-p', default=None, type=str,
                    help='Path to problem file to display via (optional)')
args = parser.parse_args()
#print args

input_answer = args.input
problem_file = args.problem
n_dims = 3
n_dims_half = n_dims / 2

# 配線を表示する
def show_board(_boards, _pboards=None, show_float=False):
    shstr = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
    idx = 0
    for z in range(board_z):
        print 'LAYER {}'.format(z + 1)
        for y in range(n_dims_half, board_y + n_dims_half):
            for x in range(n_dims_half, board_x + n_dims_half):
                # ビアを赤色で表示
                if _pboards is not None and _boards[z][y][x]['type'] == 1 and _pboards[z][y][x]['type'] == 'via':
                   sys.stdout.write('\033[1;31;47m {} \033[0m'.format(nl.int2str(_boards[z][y][x]['data'], 36)))
                elif _boards[z][y][x]['type'] == 1:
                   sys.stdout.write('\033[1;30;47m {} \033[0m'.format(nl.int2str(_boards[z][y][x]['data'], 36)))
                else:
                    # 正しい配線形状
                    fr_color = '30'

                    # 途切れてないセル / 途切れてるセル
                    bg_color = '47'
                    if show_float:
                        if _boards[z][y][x]['float'] != None:
                            bg_color = '43'

                    sys.stdout.write('\033[1;{};{}m{}\033[0m'.format(fr_color, bg_color, shstr[_boards[z][y][x]['shape']]))

                    idx = idx + 1
            print ''

board_x, board_y, board_z, boards = nl3d.read_ansfile(input_answer, n_dims)
pboards = None
if problem_file is not None:
    _, _, _, pboards = nl3d.read_probfile(problem_file, n_dims)
show_board(boards, pboards)
