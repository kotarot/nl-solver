#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
「トップスクリプト」
機械学習を用いたナンバーリンクソルバ

Step 1.
学習データを利用して大まかな答えを得る。
Fixファイルをレベルごとに生成する。
(./ml/test_window.py)

Step 2.
Fixファイルをもとに タッチアンドクロス手法
で最終的な解を求める。
(./single/solver)

参考文献
http://ja.pymotw.com/2/multiprocessing/basics.html
"""

import argparse
import multiprocessing
import subprocess
import sys
import time


# 学習データ
PICKLE = './ml/dump/s9_u200_e2000_dwindowxb_tnone.pkl'

parser = argparse.ArgumentParser(description='Top script')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Name of input file')
args = parser.parse_args()
#print args


def worker_2014():
    """ 2014手法: タッチアンドクロス """
    p = multiprocessing.current_process()
    print 'Worker 2014 Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = './single/solver --loop 500 --output A{}.txt {}.txt'.format(args.input, args.input)
    print 'Worker 2014 [0]:', cmd
    sys.stdout.flush()
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2014 Exiting :', p.name, p.pid
    sys.stdout.flush()

    return


def worker_2014fix():
    """ 2014手法: タッチアンドクロス (fix-flag) """
    p = multiprocessing.current_process()
    print 'Worker 2014 (fix-flag) Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = './single/solver --fix-flag --loop 500 --output A{}.txt {}.txt'.format(args.input, args.input)
    print 'Worker 2014 (fix-flag) [0]:', cmd
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2014 (fix-flag) Exiting :', p.name, p.pid
    sys.stdout.flush()

    return


def worker_2015(level):
    """ 2015手法: 機械学習＋タッチアンドクロス """
    p = multiprocessing.current_process()
    print 'Worker 2015 Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = 'python ./ml/test_window.py --pickle {} --level {} --output FIX{}_{}.txt {}.txt'.format(PICKLE, level, args.input, level, args.input)
    print 'Worker 2015 [0]:', cmd
    subprocess.call(cmd.strip().split(' '))

    cmd = './single/solver --fixfile FIX{}_{}.txt --loop 500 --output A{}.txt {}.txt'.format(args.input, level, args.input, args.input)
    print 'Worker 2015 [1]:', cmd
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2014 (fix-flag) Exiting :', p.name, p.pid
    sys.stdout.flush()

    return


if __name__ == '__main__':
    jobs = []

    # 2014手法: タッチアンドクロス
    p0 = multiprocessing.Process(name='a-2014', target=worker_2014)
    #p0.daemon = True
    jobs.append(p0)
    p0.start()

    # 2014手法: タッチアンドクロス (fix-flag)
    p1 = multiprocessing.Process(name='a-2014-fix', target=worker_2014fix)
    #p1.daemon = True
    jobs.append(p1)
    p1.start()

    # 2015手法: 機械学習＋タッチアンドクロス (level 0 ~ 3)
    for i in range(0, 4):
        p = multiprocessing.Process(name='a-2015-l{}'.format(i), target=worker_2015, args=(i,))
        #p.daemon = True
        jobs.append(p)
        p.start()

    # このスクリプトは最大3分実行する
    time.sleep(180)
