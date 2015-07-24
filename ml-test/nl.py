#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ナンバーリンクソルバ Utils
"""


"""
Prepare dataset
Answerファイルを読み込む

board は 上下左右に番兵を (N / 2) 行
各セルは data, type, dir 属性をもつ
* data は Ansファイル内に書いてある数字そのもの
* type は 0: blank, 1: 数字 (ターミナル), 2: 線  -- セル外と接続してる線数と等価
* shape は 配線の形 0:none 1:│ 2:┘ 3:└ 4:┐ 5:┌ 6:─
* dirb は 上下左右の配列 接続する場合 True、そうでない場合 False
* diri は 上下左右の配列 接続方向が 1 または -1 -- 1 は ソースからシンクへの経路、-1 は逆の経路
    ソースとシンクの定義: より左上にある方がソース
"""
def read_ansfile(filename, n_dims):
    board = []
    board_x, board_y = -1, -1 # ボードの X と Y
    #maxn_line = -1            # 線番号 (ラインナンバ) の最大値

    _board = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
        # 1行目以外
        else:
            line_x = [{'data': -1} for i in range(0, n_dims / 2)] + [{'data': int(token)} for token in line.split(',')] + [{'data': -1} for i in range(0, n_dims / 2)]
            _board.append(line_x)
    board = [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims / 2)] \
          + _board \
          + [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims / 2)]

    # 属性を記録する
    for y in range(n_dims / 2, board_y + n_dims / 2):
        for x in range(n_dims / 2, board_x + n_dims / 2):
            if board[y][x]['data'] != -1:
                # ラインナンバの最大値を更新
                #if maxn_line < board[y][x]['data']:
                #    maxn_line = board[y][x]['data']

                # type, shape, and dirb
                n_connect, s_connect = 0, 0
                d_connect = [False, False, False, False] # 上下左右
                connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                for i, c in enumerate(connects):
                    if board[y][x]['data'] == board[c[1]][c[0]]['data']:
                        n_connect = n_connect + 1
                        d_connect[i] = True
                board[y][x]['type']  = n_connect
                board[y][x]['dirb']  = d_connect
                if n_connect != 1:
                    if d_connect[0] == True and d_connect[1] == True:
                        board[y][x]['shape'] = 1
                    elif d_connect[0] == True and d_connect[2] == True:
                        board[y][x]['shape'] = 2
                    elif d_connect[0] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 3
                    elif d_connect[1] == True and d_connect[2] == True:
                        board[y][x]['shape'] = 4
                    elif d_connect[1] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 5
                    elif d_connect[2] == True and d_connect[3] == True:
                        board[y][x]['shape'] = 6
                    else:
                        board[y][x]['shape'] = 0

    # TODO: 経路の方向の違いによる diri の更新

    #print board
    return board_x, board_y, board


"""
データセットを生成 (配線形状の分類)
入力はターミナル数字が存在するセルを 1、存在しないセルを 0、ボード外を -1 とした (N^2 - 1) 次元のベクトル
出力は配線形状を表す分類スカラー数字
"""
def gen_dataset_shape(board_x, board_y, board, n_dims):
    x_data, y_data = [], []
    for y in range(n_dims / 2, board_y + n_dims / 2):
        for x in range(n_dims / 2, board_x + n_dims / 2):
            if board[y][x]['type'] != 1:
                dx = []
                # 入力: window
                for wy in range(-(n_dims / 2), n_dims / 2 + 1):
                    for wx in range(-(n_dims / 2), n_dims / 2 + 1):
                        if not (wx == 0 and wy == 0):
                            if board[y + wy][x + wx]['data'] == -1:
                                dx.append(-1)
                            elif board[y + wy][x + wx]['type'] == 1:
                                dx.append(1)
                            else:
                                dx.append(0)
                x_data.append(dx)
                # 出力: direction
                y_data.append(board[y][x]['shape'])

    return x_data, y_data


"""
数値型を文字列に変換する
i      数値
base   基数 (2から36の間の値）
http://lightson.dip.jp/zope/ZWiki/1742_e9_80_b2_e3_80_818_e9_80_b2_e3_80_8110_e9_80_b2_e3_80_8116_e9_80_b2_e3_81_ae_e5_90_84_e8_a1_a8_e7_8f_be_e3_82_92_e7_9b_b8_e4_ba_92_e3_81_ab_e5_a4_89_e6_8f_9b
"""
def int2str(i, base):
    int2str_table = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

    if not 2 <= base <= 36:
        raise ValueError('base must be 2 <= base < 36')

    result = []

    temp = abs(i)
    if temp == 0:
        result.append('0')
    else:
        while temp > 0:
            result.append(int2str_table[temp % base])
            temp /= base

    if i < 0:
        result.append('-')

    return ''.join(reversed(result))
