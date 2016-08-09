#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「答えを表示」
"""

import argparse
import sys

import nl


parser = argparse.ArgumentParser(description='View answer')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Path to input answer file')
args = parser.parse_args()
#print args

input_answer = args.input
n_dims = 3
n_dims_half = n_dims / 2

# 配線を表示する
def show_board(_board, show_float=False):
    shstr = ['   ', ' │ ', '─┘ ', ' └─', '─┐ ', ' ┌─', '───']
    idx = 0
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if _board[y][x]['type'] == 1:
                sys.stdout.write('\033[1;30;47m {} \033[0m'.format(nl.int2str(_board[y][x]['data'], 36)))
            else:
                # 正しい配線形状
                fr_color = '30'

                # 途切れてないセル / 途切れてるセル
                bg_color = '47'
                if show_float:
                    if _board[y][x]['float'] != None:
                        bg_color = '43'

                sys.stdout.write('\033[1;{};{}m{}\033[0m'.format(fr_color, bg_color, shstr[_board[y][x]['shape']]))

                idx = idx + 1
        print ''

board_x, board_y, board = nl.read_ansfile(input_answer, n_dims)
show_board(board)
