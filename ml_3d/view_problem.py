#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「問題を表示 3Dバージョン」
"""

import argparse
import sys

import nl
import nl3d


parser = argparse.ArgumentParser(description='View problem 3D version')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to problem file')
args = parser.parse_args()
#print args

problem_file = args.input
n_dims = 3
n_dims_half = n_dims / 2

# 問題を表示する
def show_board(_boards):
    line_num = 0
    via_num = 0
    for z in range(board_z):
        print 'LAYER {}'.format(z + 1)
        for y in range(n_dims_half, board_y + n_dims_half):
            for x in range(n_dims_half, board_x + n_dims_half):
                # ビアを赤色で表示
                if _boards[z][y][x]['type'] == 'via':
                    via_num += 1
                    if len(_boards[z][y][x]['data']) == 1:
                        sys.stdout.write('\033[1;31;47m  %s \033[0m' % _boards[z][y][x]['data'])
                    else:
                        sys.stdout.write('\033[1;31;47m %s \033[0m' % _boards[z][y][x]['data'])
                elif _boards[z][y][x]['type'] == 1:
                    line_num += 1
                    sys.stdout.write('\033[1;30;47m %02d \033[0m' % _boards[z][y][x]['data'])
                else:
                    sys.stdout.write('\033[1;30;47m    \033[0m')
            sys.stdout.write('\n')
    return line_num, via_num

board_x, board_y, board_z, boards = nl3d.read_probfile(problem_file, n_dims)
line_num, via_num = show_board(boards)
print '%d X %d X %d' % (board_x, board_y, board_z)
print '#line=%d  #via=%d' % (line_num / 2, via_num / 2)
