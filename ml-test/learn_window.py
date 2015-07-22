#!/usr/bin/env python
# -*- coding: utf-8 -*-

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F

# Prepare multi-layer perceptron model
# 多層パーセプトロン (中間層 100次元)
# 入力: 3x3 = 9次元
# 出力: 4次元
model = FunctionSet(l1=F.Linear(9, 100),
                    l2=F.Linear(100, 100),
                    l3=F.Linear(100, 4))

# Learning loop
for epoch in xrange(1, 1000):
    print 'epoch', epoch

    # Training
