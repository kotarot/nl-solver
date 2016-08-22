# -*- coding: utf-8 -*-

import conv_nl
import chainer
from chainer import optimizers
import chainer.functions as F
import chainer.links as L
import sys

import numpy as np

class Conv(chainer.Chain):

	def __init__(self):
		super(Conv, self).__init__(
			conv1 = L.Convolution2D(2, 10, 3, stride=1, pad=1),
			conv2 = L.Deconvolution2D(10, 2, 4, stride=2, pad=1)
			)

	def clear(self):
		self.loss = None
		self.accuracy = None

	def forward(self, x):
		self.clear()
		h = self.conv1(x)
		h = F.max_pooling_2d(h, 2, stride=2)
		h = self.conv2(h)
		return h

	def get_loss(self, x, t):
		self.clear()
		h = self.conv1(x)
		h = F.max_pooling_2d(h, 2, stride=2)
		h = self.conv2(h)
		loss = F.mean_squared_error(h, t)
		return loss

def main(_problem, _answer, _target=None):

	# Load a problem data
	problem = []
	for p in _problem:
		problem.append(chainer.Variable(np.asarray(p, dtype=np.float32)))

	# Load an answer data
	answer = []
	for a in _answer:
		answer.append(chainer.Variable(np.asarray(a, dtype=np.float32)))

	if _target is None:
		target = problem[0]
	else:
		target = chainer.Variable(np.asarray([_target], dtype=np.float32))

	# Create a target model
	model = Conv()
	initial = model.forward(problem[0])

	# Settings for an optimizer
	optimizer = optimizers.Adam()
	optimizer.setup(model)

	print ""
	# Learn
	for epoch in range(10000):
		if epoch % 10 == 0 :
			sys.stdout.write("\r\033[KEpoch: {0}".format(epoch))
			sys.stdout.flush()

		for i in range(len(problem)):
			loss = model.get_loss(problem[i], answer[i])
			model.zerograds()
			loss.backward()
			optimizer.update()


	trained = model.forward(target)

	res = trained.data[0]

	return res

if __name__ == '__main__':

	_dir = "../../2016-08-02_NL(problem-answer)/"
	_q_dir = "in_fol/"
	_a_dir = "out_fol/"
	_q_list = ["08", "14", "20", "26", "33", "39", "58", "64"]
	# _q_list = ["08", "14", "20", "26", "33", "39", "58", "64", "70"]

	b = []
	problem = []
	answer = []

	for v in _q_list:
		qpath = _dir + _q_dir + "NL_Q{0}.txt".format(v)
		boards = conv_nl.read_problem(qpath)
		apath = _dir + _a_dir + "T01_A{0}.txt".format(v)
		boards2 = conv_nl.read_answer(apath, problem = boards)
		
		p = conv_nl.get_matrix(boards2, ans=False)
		problem.append([p[:]])
		a = conv_nl.get_matrix(boards2, ans=True)
		answer.append([a[:]])

		for i in range(10):
			p = conv_nl.get_randomized_matrix(boards2, ans=False, seed=i)
			problem.append([p[:]])
			a = conv_nl.get_randomized_matrix(boards2, ans=True, seed=i)
			answer.append([a[:]])

	tpath = _dir + _q_dir + "NL_Q70.txt"
	boards = conv_nl.read_problem(tpath)
	tapath = _dir + _a_dir + "T01_A70.txt"
	boards2 = conv_nl.read_answer(tapath, problem = boards)
	target = conv_nl.get_matrix(boards2, ans=False)

	res = main(problem, answer, target)

	# for z in res:
	# 	for y in z:
	# 		for x in y:
	# 			if x < 0 : x = 0
	# 			sys.stdout.write("{0:3.2f} ".format(x))
	# 		sys.stdout.write("\n")
	# 	sys.stdout.write("\n")

	res = conv_nl.get_board(res)
	conv_nl.print_boards(res)

	conv_nl.print_boards(boards2)
