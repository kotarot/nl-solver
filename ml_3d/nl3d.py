#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ナンバーリンクソルバ 3Dバージョン Utils
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
    boards = []                            # 層ごとのボードの集合
    board_x, board_y, board_z = -1, -1, -1 # ボードの X, Y, Z
    #maxn_line = -1            # 線番号 (ラインナンバ) の最大値
    n_dims_half = n_dims / 2

    board_lines = []
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y, board_z = int(size[0]), int(size[1]), int(size[2])
        # 1行目以外は一旦リストに入れる
        else:
            board_lines.append(line)

    # 1行目以外の処理
    line_num = 0
    for z in range(board_z):
        _board = []
        line_num = line_num + 1 # 'LAYER 1' 等の行
        for y in range(board_y):
            # 左右の番兵含む
            line_x = [{'data': -1, 'type': -1, 'shape': -1} for i in range(0, n_dims_half)] \
                   + [{'data': int(token), 'shape': 0} for token in board_lines[line_num].split(',')] \
                   + [{'data': -1, 'type': -1, 'shape': -1} for i in range(0, n_dims_half)]
            _board.append(line_x)
            line_num = line_num + 1

        # 上下の番兵
        _board_banpei = [[{'data': -1, 'type': -1, 'shape': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)] \
                      + _board \
                      + [[{'data': -1, 'type': -1, 'shape': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)]

        boards.append(_board_banpei)
        line_num = line_num + 1 # 空行をスキップ

    # 属性を記録する
    for z in range(board_z):
        for y in range(n_dims_half, board_y + n_dims_half):
            for x in range(n_dims_half, board_x + n_dims_half):
                if boards[z][y][x]['data'] != -1:
                    # ラインナンバの最大値を更新
                    #if maxn_line < board[z][y][x]['data']:
                    #    maxn_line = board[z][y][x]['data']

                    # type, shape, and dirb
                    n_connect, s_connect = 0, 0
                    d_connect = [False, False, False, False] # 上下左右
                    connects = [[x, y - 1], [x, y + 1], [x - 1, y], [x + 1, y]] # 上下左右
                    if 0 < boards[z][y][x]['data']:
                        for i, c in enumerate(connects):
                            if boards[z][y][x]['data'] == boards[z][c[1]][c[0]]['data']:
                                n_connect = n_connect + 1
                                d_connect[i] = True
                    boards[z][y][x]['type']  = n_connect
                    boards[z][y][x]['dirb']  = d_connect
                    if n_connect != 1:
                        if d_connect[0] == True and d_connect[1] == True:
                            boards[z][y][x]['shape'] = 1
                        elif d_connect[0] == True and d_connect[2] == True:
                            boards[z][y][x]['shape'] = 2
                        elif d_connect[0] == True and d_connect[3] == True:
                            boards[z][y][x]['shape'] = 3
                        elif d_connect[1] == True and d_connect[2] == True:
                            boards[z][y][x]['shape'] = 4
                        elif d_connect[1] == True and d_connect[3] == True:
                            boards[z][y][x]['shape'] = 5
                        elif d_connect[2] == True and d_connect[3] == True:
                            boards[z][y][x]['shape'] = 6
                        else:
                            boards[z][y][x]['shape'] = 0

    # TODO: 経路の方向の違いによる diri の更新

    return board_x, board_y, board_z, boards


"""
Prepare dataset
Problemファイルを読み込む
"""
def read_probfile(filename, n_dims):
    boards = []
    board_x, board_y = -1, -1 # ボードの X と Y
    n_dims_half = n_dims / 2

    _boards = None
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y, board_z = int(size[0]), int(size[1]), int(size[2])
            # ボードサイズで初期化
            _boards = [[[{'data': 0, 'type': 2, 'shape': -1} for x in range(0, board_x)] for y in range(0, board_y)] for z in range(0, board_z)]
        # 2行目
        elif 'LINE_NUM' in line:
            continue
        # 3行目以降 (LINE)
        elif 'LINE' in line or 'Line' in line:
            line = line.replace('(', '').replace(')', '')
            tokens = line.split(' ')
            line_tokens = tokens[0].split('#')
            line_id = int(line_tokens[1])
            src = tokens[1].split(',')
            snk = tokens[2].split(',')
            _boards[int(src[2]) - 1][int(src[1])][int(src[0])] = {'data': line_id, 'type': 1, 'shape': 0}
            _boards[int(snk[2]) - 1][int(snk[1])][int(snk[0])] = {'data': line_id, 'type': 1, 'shape': 0}
        # 3行目以降 (VIA)
        elif 'VIA' in line or 'Via' in line:
            line = line.replace('(', '').replace(')', '')
            tokens = line.split(' ')
            line_tokens = tokens[0].split('#')
            via_id = line_tokens[1]
            src = tokens[1].split(',')
            snk = tokens[2].split(',')
            _boards[int(src[2]) - 1][int(src[1])][int(src[0])] = {'data': via_id, 'type': 'via', 'shape': 0}
            _boards[int(snk[2]) - 1][int(snk[1])][int(snk[0])] = {'data': via_id, 'type': 'via', 'shape': 0}

    # 左右の番兵
    for z in range(0, board_z):
        for y in range(0, board_y):
            _boards[z][y] = [{'data': -1, 'type': -1, 'shape': -1} for i in range(0, n_dims_half)] \
                          + _boards[z][y] \
                          + [{'data': -1, 'type': -1, 'shape': -1} for i in range(0, n_dims_half)]

    # 上下の番兵
    for z in range(0, board_z):
        boards.append([])
        boards[z] = [[{'data': -1, 'type': -1, 'shape': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)] \
                  + _boards[z] \
                  + [[{'data': -1, 'type': -1, 'shape': -1} for i in range(0, board_x + n_dims - 1)] for j in range(0, n_dims_half)]

    return board_x, board_y, board_z, boards


"""
個数を正規化する
"""
def norm_number(n):
    if n <= 0:
        return 0
    elif n <= 1:
        return 0.5
    else:
        return 1


"""
データセットを生成 (2016年 ビアdistance-and-direction)
入力: ビアを2、ターミナル数字が存在するセルを 1、存在しないセルを 0、ボード外を -1 とした (N^2 - 1) 次元のベクトル
出力: どのエリアのビアを使用するか

[dataset]
  - dd4: (入力) window + 同じ数字 --> (出力) 4方向
  - dd8: (入力) window + 同じ数字 --> (出力) 4方向 + 距離 (window内か外か)
  - ddx8: (入力) window + 同じ数字 + 他方層の相対位置 --> 4方向 + 距離 (window内か外か)
"""
def gen_dataset_dd(board_x, board_y, board_z, boards, arg_z, n_dims, dataset, testmode=False):
    board = boards[arg_z]
    x_data, y_data = [], []
    n_dims_half = n_dims / 2

    assert(dataset == 'dd4' or dataset == 'dd8' or dataset == 'ddx8')
    for y in range(n_dims_half, board_y + n_dims_half):
        for x in range(n_dims_half, board_x + n_dims_half):
            # ターミナル数字セルの場合
            # かつ、もう一方のセルが同じ層に無い（別層にある）場合
            # のみ対象
            if board[y][x]['type'] == 1:
                count_terms = 0
                for ay in range(n_dims_half, board_y + n_dims_half):
                    for ax in range(n_dims_half, board_x + n_dims_half):
                        if board[ay][ax]['type'] == 1 and board[ay][ax]['data'] == board[y][x]['data']:
                            count_terms += 1
                assert(count_terms == 1 or count_terms == 2)
                if count_terms == 2:
                    continue

                # 別層 (他方層) にある同じ数字の別端点の座標
                if dataset == 'ddx8':
                    terms = []
                    for tz in range(board_z):
                        for ty in range(n_dims_half, board_y + n_dims_half):
                            for tx in range(n_dims_half, board_x + n_dims_half):
                                if boards[tz][ty][tx]['type'] == 1 and boards[tz][ty][tx]['data'] == board[y][x]['data']:
                                    terms.append( {'x': tx, 'y': ty, 'z': tz} );
                    #print board[y][x]['data'], terms
                    assert(len(terms) == 2)
                    if terms[0]['x'] == x and terms[0]['y'] == y:
                        alt_x = terms[1]['x']
                        alt_y = terms[1]['y']
                    elif terms[1]['x'] == x and terms[1]['y'] == y:
                        alt_x = terms[0]['x']
                        alt_y = terms[0]['y']
                    else:
                        assert(False)

                # 入力: window
                dx = []
                ones = [] # 1回現れた数字のリスト
                twos = [] # 2回現れた数字のリスト
                if dataset == 'dd4' or dataset == 'dd8' or dataset == 'ddx8':
                    # 自セルの周囲
                    for wy in range(-n_dims_half, n_dims_half + 1):
                        for wx in range(-n_dims_half, n_dims_half + 1):
                            if not (wx == 0 and wy == 0):
                                cellx = 0.0
                                if board[y + wy][x + wx]['data'] == -1:
                                    cellx = -1.0
                                elif board[y + wy][x + wx]['type'] == 1:
                                    cellx = 0.5
                                    if board[y + wy][x + wx]['data'] not in ones:
                                        ones.append(board[y + wy][x + wx]['data'])
                                    else:
                                        twos.append(board[y + wy][x + wx]['data'])
                                elif board[y + wy][x + wx]['type'] == 'via':
                                    cellx = 1.0
                                else:
                                    cellx = 0.0
                                dx.append(cellx)
                    # 同じ数字の数 (上下/左右にまたがる)
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
                else:
                    raise NotImplementedError()
                if dataset == 'ddx8':
                    # board_width = max(board_x, board_y)
                    dist_x = 1.0 * (alt_x - x) / board_x
                    # dist_x = 1.0 * (alt_x - x) / board_width
                    #dist_x = 1.0 * (alt_x - x) / board_width * 2.0
                    #if dist_x < 0.0:
                    #    dist_x = max(dist_x, -1.0)
                    #else:
                    #    dist_x = min(dist_x, 1.0)
                    dist_y = 1.0 * (alt_y - y) / board_y
                    # dist_y = 1.0 * (alt_y - y) / board_width
                    #dist_y = 1.0 * (alt_y - y) / board_width * 2.0
                    #if dist_y < 0.0:
                    #    dist_y = max(dist_y, -1.0)
                    #else:
                    #    dist_y = min(dist_y, 1.0)
                    dx.append(dist_x)
                    dx.append(dist_y)
                #print dx
                x_data.append(dx)

                # 出力
                if testmode:
                    y_data = []
                elif dataset == 'dd4':
                    # 対応ビア位置エリア (右上: 0, 左上: 1, 左下: 2, 右下: 3)
                    #  ※ Yは正規化してはいけなくて整数じゃないと正確に学習できない
                    dy = -1
                    found = False
                    for vy in range(n_dims_half, board_y + n_dims_half):
                        for vx in range(n_dims_half, board_x + n_dims_half):
                            if board[vy][vx]['type'] == 'via' and board[y][x]['data'] == board[vy][vx]['data']:
                                if vy <= y:
                                    if vx <= x:
                                        dy = 1
                                    else:
                                        dy = 0
                                else:
                                    if vx <= x:
                                        dy = 2
                                    else:
                                        dy = 3
                                found = True
                                break
                        if found:
                            break
                    assert(-1 < dy)
                    y_data.append(dy)
                elif dataset == 'dd8' or dataset == 'ddx8':
                    # 対応ビア位置エリア (右上近: 0, 右上遠: 1, 左上近: 2,左上遠: 3, 左下近: 4, 左下遠: 5, 右下近: 6, 右下遠: 7)
                    #  ※ Yは正規化してはいけなくて整数じゃないと正確に学習できない
                    dy = -1
                    found = False
                    for vy in range(n_dims_half, board_y + n_dims_half):
                        for vx in range(n_dims_half, board_x + n_dims_half):
                            if board[vy][vx]['type'] == 'via' and board[y][x]['data'] == board[vy][vx]['data']:
                                if vy <= y:
                                    if vx <= x:
                                        if mdist(vy, y, vx, x) <= n_dims:
                                            dy = 2
                                        else:
                                            dy = 3
                                    else:
                                        if mdist(vy, y, vx, x) <= n_dims:
                                            dy = 0
                                        else:
                                            dy = 1
                                else:
                                    if vx <= x:
                                        if mdist(vy, y, vx, x) <= n_dims:
                                            dy = 4
                                        else:
                                            dy = 5
                                    else:
                                        if mdist(vy, y, vx, x) <= n_dims:
                                            dy = 6
                                        else:
                                            dy = 7
                                found = True
                                break
                        if found:
                            break
                    assert(-1 < dy)
                    y_data.append(dy)
                else:
                    raise NotImplementedError()

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

    if 36 <= i:
        return '#'

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


"""
マンハッタン距離
"""
def mdist(x1, y1, x2, y2):
    return abs(x1 - x2) + abs(y1 - y2)
