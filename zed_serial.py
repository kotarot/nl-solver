#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
ZedBoardとシリアル通信

参考文献
http://qiita.com/kosystem/items/0023cfee941fdf099087
"""

import serial

# 入力書き込み
ser = serial.Serial(port='COM5', baudrate=115200, timeout=1.0)
ser.write('0\n')
#ser.close()

# 出力読み込み
#ser = serial.Serial(port='COM5', baudrate=115200, timeout=1.0)
#c = ser.read()  # 1文字読み込み
str = ser.read(1000)  # 指定も字数読み込み ただしtimeoutが設定されている婆は読み取れた分だけ
#line = ser.readline()  # 行終端'¥n'までリードする
ser.close()

print str
