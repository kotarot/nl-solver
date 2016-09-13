#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ZedBoardとシリアル通信したり問題ファイルをZynqに与える形式に変換したり

参考文献
http://qiita.com/kosystem/items/0023cfee941fdf099087
"""

import argparse
import serial


parser = argparse.ArgumentParser(description='zed')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Name of input problem file')
args = parser.parse_args()
print args


"""
Problemファイルを読み込む
"""
def read_probfile(filename):
    boardstr = ''

    via_cnt = 1
    for line in open(filename, 'r'):
        # 1行目
        if 'SIZE'in line:
            tokens = line.split(' ')
            size = tokens[1].split('X')
            board_x, board_y, board_z = int(size[0]), int(size[1]), int(size[2])
            boardstr = 'X%02dY%02dZ%d' % (board_x, board_y, board_z)
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
            boardstr += 'L%02d%02d%02d%d%02d%02d%d' % (line_id, int(src[0]), int(src[1]), int(src[2]), int(snk[0]), int(snk[1]), int(snk[2]))
        # 3行目以降 (VIA)
        elif 'VIA' in line or 'Via' in line:
            line = line.replace('(', '').replace(')', '')
            tokens = line.split(' ')
            line_tokens = tokens[0].split('#')
            via_id = line_tokens[1]
            src = tokens[1].split(',')
            snk = tokens[-1].split(',')
            boardstr += 'V%02d%02d%02d%d%02d%02d%d' % (via_cnt, int(src[0]), int(src[1]), int(src[2]), int(snk[0]), int(snk[1]), int(snk[2]))
            via_cnt += 1

    return boardstr


if __name__ == '__main__':
    boardstr = read_probfile(args.input)
    print boardstr

    # 入力書き込み
    ser = serial.Serial(port='COM5', baudrate=115200, timeout=1.0)
    ser.write(boardstr + '\n')
    #ser.close()

    # 出力読み込み
    #ser = serial.Serial(port='COM5', baudrate=115200, timeout=1.0)
    #c = ser.read()  # 1文字読み込み
    str = ser.read(1000)  # 指定も字数読み込み ただしtimeoutが設定されている婆は読み取れた分だけ
    #line = ser.readline()  # 行終端'¥n'までリードする
    ser.close()

    print str
