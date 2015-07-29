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
    n_dims_half = n_dims / 2

    _board = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
        # 1行目以外
        else:
            # 左右の番兵含む
            line_x = [{'data': -1} for i in range(0, n_dims_half)] \
                   + [{'data': int(token)} for token in line.split(',')] \
                   + [{'data': -1} for i in range(0, n_dims_half)]
            _board.append(line_x)

    # 上下の番兵
    board = [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)] \
          + _board \
          + [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)]

    # 属性を記録する
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if board[y][x]['data'] != -1:
                # ラインナンバの最大値を更新
                #if maxn_line < board[y][x]['data']:
                #    maxn_line = board[y][x]['data']

                # type, shape, and dirb
                n_connect, s_connect = 0, 0
                d_connect = [False, False, False, False] # 上下左右
                connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                if 0 < board[y][x]['data']:
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

    return board_x, board_y, board


"""
Prepare dataset
Problemファイルを読み込む
"""
def read_probfile(filename, n_dims):
    board = []
    board_x, board_y = -1, -1 # ボードの X と Y
    n_dims_half = n_dims / 2

    _board = None
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y = int(size[0]), int(size[1])
            # ボードサイズで初期化
            _board = [[{'data': 0, 'type': -1} for x in range(0, board_x)] for y in range(0, board_y)]
        # 2行目
        elif 'LINE_NUM' in line:
            continue
        # 3行目以降
        else:
            line = line.replace('(', '').replace(')', '')
            tokens = line.split(' ')
            line_tokens = tokens[0].split('#')
            n_line = int(line_tokens[1])
            point_tokens = tokens[1].split('-')
            src = point_tokens[0].split(',')
            snk = point_tokens[1].split(',')
            _board[int(src[1])][int(src[0])] = {'data': n_line, 'type': 1}
            _board[int(snk[1])][int(snk[0])] = {'data': n_line, 'type': 1}

    # 左右の番兵
    for y in range(0, board_y):
        _board[y] = [{'data': -1} for i in range(0, n_dims_half)] \
                  + _board[y] \
                  + [{'data': -1} for i in range(0, n_dims_half)]

    # 上下の番兵
    board = [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)] \
          + _board \
          + [[{'data': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)]

    return board_x, board_y, board


"""
個数を正規化する
"""
def norm_number(n):
    if n <= 0:
        return -1
    elif n <= 1:
        return 0
    else:
        return 1


"""
データセットを生成 (配線形状の分類)
入力はターミナル数字が存在するセルを 1、存在しないセルを 0、ボード外を -1 とした (N^2 - 1) 次元のベクトル
出力は配線形状を表す分類スカラー数字

[dataset]
  - window   : 自セルの周囲
  - windowsn : 自セルの周囲 + 同じ数字の数
  - windowxa : 自セルの周囲 + 同じ数字の数 + 上下/左右にまたがる同じ数字の数
  - windowxb : 自セルの周囲 + 上下/左右にまたがる同じ数字の数
"""
def gen_dataset_shape(board_x, board_y, board, n_dims, dataset):
    x_data, y_data = [], []
    n_dims_half = n_dims / 2

    assert(dataset == 'window' or dataset == 'windowsn' or dataset == 'windowxa' or dataset == 'windowxb')
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            if board[y][x]['type'] != 1:
                # 入力: window
                dx = []
                ones = [] # 1回現れた数字のリスト
                twos = [] # 2回現れた数字のリスト
                # 自セルの周囲
                for wy in range(-n_dims_half, n_dims_half + 1):
                    for wx in range(-n_dims_half, n_dims_half + 1):
                        if not (wx == 0 and wy == 0):
                            cellx = 0
                            if board[y + wy][x + wx]['data'] == -1:
                                cellx = -1
                            elif board[y + wy][x + wx]['type'] == 1:
                                cellx = 1
                                if board[y + wy][x + wx]['data'] not in ones:
                                    ones.append(board[y + wy][x + wx]['data'])
                                else:
                                    twos.append(board[y + wy][x + wx]['data'])
                            else:
                                cellx = 0
                            dx.append(cellx)
                # 同じ数字の数
                if dataset == 'windowsn' or dataset == 'windowxa':
                    dx.append(norm_number(len(twos)))
                # 同じ数字の数 (上下/左右にまたがる)
                if dataset == 'windowxa' or dataset == 'windowxb':
                    cross_ver, cross_hor = 0, 0
                    for n in twos:
                        points = []
                        for wy in range(-n_dims_half, n_dims_half + 1):
                            for wx in range(-n_dims_half, n_dims_half + 1):
                                if board[y + wy][x + wx]['data'] == n and board[y + wy][x + wx]['type'] == 1:
                                    points.append({'wx': wx, 'wy': wy})
                        assert(len(points) == 2)
                        if points[0]['wy'] * points[1]['wy'] <= 0: # 上下またがる
                            cross_ver = cross_ver + 1
                        if points[0]['wx'] * points[1]['wx'] <= 0: # 左右またがる
                            cross_hor = cross_hor + 1
                    dx.append(norm_number(cross_ver))
                    dx.append(norm_number(cross_hor))
                x_data.append(dx)

                # 出力: direction
                if 'shape' in board[y][x]:
                    y_data.append(board[y][x]['shape'])

    return x_data, y_data


"""
データセットを生成
配線接続位置の分類 (ソースから)
  -- 未実装
"""
def gen_dataset_dirsrc(board_x, board_y, board, n_dims):
    raise NotImplementedError()


"""
データセットを生成
配線接続位置の分類 (シンクから)
  -- 未実装
"""
def gen_dataset_dirsnk(board_x, board_y, board, n_dims):
    raise NotImplementedError()


"""
数値型を文字列に変換する
i      数値
base   基数 (2から36の間の値）
http://lightson.dip.jp/zope/ZWiki/1742_e9_80_b2_e3_80_818_e9_80_b2_e3_80_8110_e9_80_b2_e3_80_8116_e9_80_b2_e3_81_ae_e5_90_84_e8_a1_a8_e7_8f_be_e3_82_92_e7_9b_b8_e4_ba_92_e3_81_ab_e5_a4_89_e6_8f_9b
"""
def int2str(i, base):
    int2str_table = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

    if not 2 <= base <= 36:
        raise ValueError('base must be 2 <= base <= 36')

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
