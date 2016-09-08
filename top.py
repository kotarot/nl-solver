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


parser = argparse.ArgumentParser(description='Top script')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Name of input file')
parser.add_argument('--size', '-s', default=9, type=int,
                    help='Window size (default: 9)')
args = parser.parse_args()
#print args

# 学習データ
# PICKLE = './ml/dump/s{}_u200_e2000_dwindowxb_tnone.pkl'.format(args.size)
PICKLE = './ml_3d/dump/s{}_u200_e1000_mddv8_tT01A06.pkl'.format(args.size)


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

    print 'Worker 2015 (fix-flag) Exiting :', p.name, p.pid
    sys.stdout.flush()

    return

def worker_2016():
    """ 2016手法: タッチアンドクロス3D """
    p = multiprocessing.current_process()
    print 'Worker 2016 Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = './single_3d/solver --loop 500 --output A{}.txt {}.txt'.format(args.input, args.input)
    print 'Worker 2016 [1]:', cmd
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2016 Exiting :', p.name, p.pid
    sys.stdout.flush()

    return

def worker_2016_ml():
    """ 2016手法: 機械学習3D＋タッチアンドクロス3D """
    p = multiprocessing.current_process()
    print 'Worker 2016 (with ml) Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = 'python ./ml_3d/test_3d.py --pickle {} --output {}_ml.txt {}.txt'.format(PICKLE, args.input, args.input)
    print 'Worker 2016 (with ml) [0]:', cmd
    subprocess.call(cmd.strip().split(' '))

    cmd = './single_3d/solver --loop 500 --output A{}.txt {}_ml.txt'.format(args.input, args.input)
    print 'Worker 2016 (with ml) [1]:', cmd
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2016 (with ml) Exiting :', p.name, p.pid
    sys.stdout.flush()

    return


if __name__ == '__main__':
    jobs = []

    # 2016手法: タッチアンドクロス3D
    for i in range(0, 4):
        p = multiprocessing.Process(name='a-2016-{}'.format(i), target=worker_2016)
        #p.daemon = True
        jobs.append(p)
        p.start()

    # 2016手法: 機械学習3D＋タッチアンドクロス3D
    for i in range(0, 4):
        p = multiprocessing.Process(name='a-2016-ml-{}'.format(i), target=worker_2016_ml)
        #p.daemon = True
        jobs.append(p)
        p.start()

    # このスクリプトは最大10分実行する
    time.sleep(600)
