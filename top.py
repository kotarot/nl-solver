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
import os
import shutil
import subprocess
import sys
import time

sys.path.append('../conmgr/server')
import nlcheck
nlc = nlcheck.NLCheck()

parser = argparse.ArgumentParser(description='Top script')
parser.add_argument('input', nargs=None, default=None, type=str,
                    help='Name of input file')
parser.add_argument('--size', '-s', default=9, type=int,
                    help='Window size (default: 9)')
args = parser.parse_args()
#print args

# 学習データ
# PICKLE = './ml/dump/s{}_u200_e2000_dwindowxb_tnone.pkl'.format(args.size)
PICKLE = './ml_3d/dump/s{}_u500_e2000_mddv8_tnone.pkl'.format(args.size)


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

def worker_2016(i, results):
    """ 2016手法: タッチアンドクロス3D """
    p = multiprocessing.current_process()
    print 'Worker 2016 Starting:', p.name, p.pid
    sys.stdout.flush()

    time.sleep((i+1)/1000.0)

    cmd = './single_3d/solver --output A{}_{}.txt {}.txt'.format(args.input, i, args.input)
    print 'Worker 2016 [1]:', cmd
    out = subprocess.check_output(cmd.strip().split(' '))

    print 'Worker 2016 Exiting :', p.name, p.pid
    #sys.stdout.flush()
    print out

    # ベスト解答ファイルと品質を比較する
    judges_this = nlc.check(nlc.read_input_file('{}.txt'.format(args.input)), nlc.read_target_file('A{}_{}.txt'.format(args.input, i)))
    print judges_this
    sys.stdout.flush()
    if not os.path.isfile('T03_A%s.txt' % (args.input[4:6])):
        shutil.copyfile('A{}_{}.txt'.format(args.input, i), 'T03_A%s.txt' % (args.input[4:6]))
    else:
        judges = nlc.check(nlc.read_input_file('{}.txt'.format(args.input)), nlc.read_target_file('T03_A%s.txt' % args.input[4:6]))
        print judges
        sys.stdout.flush()
        if judges_this[0][0] and judges[0][1] < judges_this[0][1]:
            shutil.copyfile('A{}_{}.txt'.format(args.input, i), 'T03_A%s.txt' % (args.input[4:6]))
    results['A{}_{}.txt'.format(args.input, i)] = judges_this

    return

def worker_2016_ml(th, results):
    """ 2016手法: 機械学習3D＋タッチアンドクロス3D """
    p = multiprocessing.current_process()
    print 'Worker 2016 (with ml) Starting:', p.name, p.pid
    sys.stdout.flush()

    cmd = 'python ./ml_3d/test_3d.py --pickle {} --output {}_ml.txt --threshold {} {}.txt'.format(PICKLE, args.input, th, args.input)
    print 'Worker 2016 (with ml) [0]:', cmd
    subprocess.call(cmd.strip().split(' '))

    cmd = './single_3d/solver --output A{}ml_{}.txt {}_ml.txt'.format(args.input, th, args.input)
    print 'Worker 2016 (with ml) [1]:', cmd
    subprocess.call(cmd.strip().split(' '))

    print 'Worker 2016 (with ml) Exiting :', p.name, p.pid
    sys.stdout.flush()

    # ベスト解答ファイルと品質を比較する
    judges_this = nlc.check(nlc.read_input_file('{}_ml.txt'.format(args.input)), nlc.read_target_file('A{}ml_{}.txt'.format(args.input, th)))
    print judges_this
    sys.stdout.flush()
    if not os.path.isfile('T03_A%s.txt' % (args.input[4:6])):
        shutil.copyfile('A{}ml_{}.txt'.format(args.input, th), 'T03_A%s.txt' % (args.input[4:6]))
    else:
        judges = nlc.check(nlc.read_input_file('{}_ml.txt'.format(args.input)), nlc.read_target_file('T03_A%s.txt' % args.input[4:6]))
        print judges
        sys.stdout.flush()
        if judges_this[0][0] and judges[0][1] < judges_this[0][1]:
            shutil.copyfile('A{}ml_{}.txt'.format(args.input, th), 'T03_A%s.txt' % (args.input[4:6]))
    results['A{}ml_{}.txt'.format(args.input, th)] = judges_this

    return


# (その時点で) 最も良い答えを採用する
def post_proc(results):
    print ''
    print 'Top script Finished!'
    print results

    if len(results) != 0:
        best_answer = None
        best_quality = -1.0
        for fn, result in results.items():
            if best_quality < result[0][1]:
                best_answer = fn
                best_quality = result[0][1]

        print 'Best answer: %s (%lf)' % (best_answer, best_quality)
        shutil.copyfile(best_answer, 'T03_A%s.txt' % (best_answer[5:7]))


if __name__ == '__main__':
        jobs = []
        manager = multiprocessing.Manager()
        results = manager.dict()

    # Ctrl-C シグナルを捕まえても終了処理をする
#    try:

        # 2016手法: タッチアンドクロス3D
        for i in range(0, 2):
            p = multiprocessing.Process(name='a-2016-{}'.format(i), target=worker_2016, args=(i, results))
            #p.daemon = True
            jobs.append(p)
            p.start()
            #p.join()

        # 2016手法: 機械学習3D＋タッチアンドクロス3D
        ths = [0, 5]
        for i in range(0, 2):
            p = multiprocessing.Process(name='a-2016-ml-{}'.format(i), target=worker_2016_ml, args=(ths[i], results))
            #p.daemon = True
            jobs.append(p)
            p.start()
            #p.join()

        # このスクリプトは最大10分実行する
        time.sleep(600)

        #post_proc(results)

#    except:
#        print 'Exception!'
#        raise
#
#    finally:
#        post_proc(results)
