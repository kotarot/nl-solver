#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
数値型を文字列に変換する。
i      数値
base   基数 (2から36の間の値）
http://lightson.dip.jp/zope/ZWiki/1742_e9_80_b2_e3_80_818_e9_80_b2_e3_80_8110_e9_80_b2_e3_80_8116_e9_80_b2_e3_81_ae_e5_90_84_e8_a1_a8_e7_8f_be_e3_82_92_e7_9b_b8_e4_ba_92_e3_81_ab_e5_a4_89_e6_8f_9b
"""

int2str_table = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

def int2str(i, base):
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
