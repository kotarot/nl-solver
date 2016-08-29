# -*- encoding: utf-8 -*-

import numpy as np
from chainer import cuda, Function, FunctionSet, gradient_check, Variable, optimizers
import chainer.functions as F


class Model1:

    def __init__(self, model):
        if isinstance(model, tuple):
            input_dims, n_units, output_dims = model
            self.model = FunctionSet(l1=F.Linear(input_dims, n_units),
                                     l2=F.Linear(n_units, n_units),
                                     l3=F.Linear(n_units, output_dims))
        else:
            self.model = model

    def __call__(self):
        return self.model

    # Neural net architecture
    # ニューラルネットの構造
    def forward(self, x_data, y_data, train=True):
        x = Variable(x_data)
        if not y_data is None: t = Variable(y_data)
        h1 = F.dropout(F.relu(self.model.l1(x)),  train=train)
        h2 = F.dropout(F.relu(self.model.l2(h1)), train=train)
        y  = self.model.l3(h2)
        if not y_data is None:
            # 多クラス分類なので誤差関数としてソフトマックス関数の
            # 交差エントロピー関数を用いて、誤差を導出
            return F.softmax_cross_entropy(y, t), F.accuracy(y, t), y
        else:
            return y

    def evaluate(self, x_data):
        return self.forward(x_data, None, train=False)

class Model2:

    def __init__(self, model):
        if isinstance(model, tuple):
            input_dims, n_units, output_dims = model
            self.model = FunctionSet(l1=F.Linear(input_dims, n_units),
                                     l2=F.Linear(n_units, n_units/2),
                                     l3=F.Linear(n_units/2, output_dims))
        else:
            self.model = model

    def __call__(self):
        return self.model

    # Neural net architecture
    # ニューラルネットの構造
    def forward(self, x_data, y_data, train=True):
        x = Variable(x_data)
        if not y_data is None: t = Variable(y_data)
        h1 = F.dropout(F.relu(self.model.l1(x)),  train=train)
        h2 = F.dropout(F.relu(self.model.l2(h1)), train=train)
        y  = self.model.l3(h2)
        if not y_data is None:
            # 多クラス分類なので誤差関数としてソフトマックス関数の
            # 交差エントロピー関数を用いて、誤差を導出
            return F.softmax_cross_entropy(y, t), F.accuracy(y, t), y
        else:
            return y

    def evaluate(self, x_data):
        return self.forward(x_data, None, train=False)
