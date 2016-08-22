# -*- encoding: utf-8 -*-

import os
import sys
import re
import random

debug = False

def read_problem(fn):
	"""
	Read a problem file.
	@param fn	A problem file name to read.
	@return		A list of boards[z][y][x]
	"""

	if debug:
		print "Read problem : ", fn
	fp = open(fn, 'r')
	txts = fp.readlines()
	fp.close()

	bs = (0, 0, 0)	# Board size: x, y, z
	ln = 0			# The number of lines
	lines = dict()		# A dictionary of lines
	vias = dict()		# A dictionary of vias

	for _l in txts:

		_l = _l.replace("\n", "")
		_l = _l.replace("\r", "")

		if _l.upper().startswith("SIZE"):
			r = re.compile("^SIZE (?P<size_x>[0-9]+)X(?P<size_y>[0-9]+)X(?P<size_z>[0-9]+)$", re.IGNORECASE)
			m = r.search(_l)

			bs = (int(m.group("size_x")), int(m.group("size_y")), int(m.group("size_z")))

			if debug:
				print "SIZE(org)\t", m.groups()
				print "SIZE(bs)\t", bs

		elif _l.upper().startswith("LINE_NUM"):
			r = re.compile("^LINE_NUM (?P<linen>[0-9]+)$", re.IGNORECASE)
			m = r.search(_l)

			ln = m.group("linen")

			if debug:
				print "LINE_NUM(org)\t", m.group(1)
				print "LINE_NUM(ln)\t", ln

		elif _l.upper().startswith("LINE"):
			r = re.compile("^LINE#([0-9]+) \(([0-9]+),([0-9]+),([0-9]+)\) \(([0-9]+),([0-9]+),([0-9]+)\)$", re.IGNORECASE)
			m = r.search(_l)

			line_1 = (int(m.group(2)),int(m.group(3)),int(m.group(4)))
			line_2 = (int(m.group(5)),int(m.group(6)),int(m.group(7)))
			dat = [line_1, line_2]
			_key = m.group(1)
			lines[_key] = dat[:]

			if debug:
				print "LINE\t", m.groups()
				print "LINE\t", dat

		elif _l.upper().startswith("VIA"):
			r = re.compile("^VIA#([a-zA-Z]+) (.*)$")
			m = r.search(_l)

			if m:
				r = re.compile("\(([0-9]+),([0-9]+),([0-9]+)\)")
				m2 = r.findall(m.group(2))
				dat = []
				for v in m2:
					dat.append((int(v[0]), int(v[1]), int(v[2])))
				_key = m.group(1)
				vias[_key] = dat[:]

				if debug:
					print m.group(1), "\t", m2
					print m.group(1), "\t", dat

		else:
			pass

	boards = [[[{'type':0, 'value':None} for x in range(0, bs[0])] for y in range(0, bs[1])] for z in range(0, bs[2])]

	for k, v in lines.items():
		for _l in v:
			boards[_l[2]-1][_l[1]][_l[0]] = {'type': 1, 'value': k}

	for k, v in vias.items():
		for _l in v:
			boards[_l[2]-1][_l[1]][_l[0]] = {'type': 2, 'value': k}

	return boards

def read_answer(fn, problem=None):
	"""
	Read a problem file.
	@param fn	A problem file name to read.
	@return		A list of boards[z][y][x]
	"""

	if debug:
		print "Read answer : ", fn
	fp = open(fn, 'r')
	txts = fp.readlines()
	fp.close()

	bs = (0, 0, 0)	# Board size: x, y, z
	ln = 0			# The number of lines
	lines = dict()		# A dictionary of lines
	vias = dict()		# A dictionary of vias

	boards = []
	_z = None
	_y = None

	for _l in txts:

		if _l.upper().startswith("SIZE"):
			r = re.compile("^SIZE (?P<size_x>[0-9]+)X(?P<size_y>[0-9]+)X(?P<size_z>[0-9]+)$", re.IGNORECASE)
			m = r.search(_l)

			bs = (int(m.group("size_x")), int(m.group("size_y")), int(m.group("size_z")))
			# expand x, y with a grid for subsecuent processes
			boards = [[[{'type':0, 'value':None} for x in range(0, bs[0]+1)] for y in range(0, bs[1]+1)] for z in range(0, bs[2])]

			if debug:
				print "SIZE(org)\t", m.groups()
				print "SIZE(bs)\t", bs

		elif _l.upper().startswith("LAYER"):
			r = re.compile("^LAYER (?P<layer>[0-9]+)$", re.IGNORECASE)
			m = r.search(_l)

			_z = int(m.group("layer"))
			_y = 0

			if debug:
				print "LAYER(org)\t", m.group(1)
				print "LAYER(ln)\t", _z

		else:
			dat = _l.rstrip().split(',')

			if len(dat) != bs[0]:
				continue

			for _x, v in enumerate(dat):
				# if v == '0':
				if v.isdigit() and int(v)==0:
					_type = 0
				else:
					_type = 3
				boards[_z-1][_y][_x] = {'type': _type, 'value': v}

			_y += 1

	for z, zv in enumerate(boards):
		for y, yv in enumerate(zv):
			for x, xv in enumerate(yv):
				if boards[z][y][x]['type'] == 3:
					_v = boards[z][y][x]['value']
					__n = boards[z][y-1][x]['value'] == _v
					__e = boards[z][y][x+1]['value'] == _v
					__s = boards[z][y+1][x]['value'] == _v
					__w = boards[z][y][x-1]['value'] == _v

					_sum = __n + __e + __s + __w

					if _sum <= 1:
						if (not problem is None):
							boards[z][y][x]['type'] = problem[z][y][x]['type']
					elif _sum == 2:
						_type = __n + __e*2 + __s*4
						boards[z][y][x]['type'] = 3 + _type

			boards[z][y].pop()
		boards[z].pop()

	return boards

def print_boards(b):
	lcnt = 1
	junctions = ['', '', '', '', '┘', '─', '└', '┐', '│', '┌']
	for z in b:
		print "LAYER ", lcnt
		for y in z:
			for x in y:
				buf = ""
				if x['type'] == 0:
					buf = '   '
				elif x['type'] >= 4:
					buf = ' ' + junctions[x['type']] + ' '
				else:
					buf = str(x['value'])
					if len(buf) == 1:
						buf = ' ' + buf + ' '
					elif len(buf) == 2:
						buf = ' ' + buf
					if x['type'] == 2:
						buf = "\033[31m" + buf
				sys.stdout.write("\033[30m\033[47m"+buf)
			sys.stdout.write("\033[39m\033[49m\n")
		lcnt+=1
		print ""

def get_matrix(b, ans=False):
	res = []
	for z in b:
		tmpz = []
		for y in z:
			tmpy = []
			for x in y:
				if x['type'] == 0:
					tmpy.append(0)
				elif x['type'] >= 4:
					if ans:
						# tmpy.append(0.6 + (x['type']-4)/20.0)
						tmpy.append(0.125 + (x['type']-4)*0.125)
					else:
						tmpy.append(0)
				else:
					if ans:
						if x['type'] == 2:
							tmpy.append(0.90)
						else:
							tmpy.append(0.95)
					else:
						if x['type'] == 2:
							# via
							tmpy.append(0.4 + int(x['value'])/500.0)
						else:
							# line
							tmpy.append(0.2 + int(x['value'])/500.0)
			if len(tmpy) % 2 == 1:
				tmpy.append(0)
			tmpz.append(tmpy[:])
		res.append(tmpz[:])
	return res

def get_board(mat, ans=False):
	res = []
	for z in mat:
		tmpz = []
		for y in z:
			tmpy = []
			for x in y:
				if ans:
					if x < 0.0625:
						tmpy.append({'type':0, 'value':0})
					elif x < 0.85:
						tmpy.append({'type': int((x-0.125)*8+4), 'value':0})
					elif x < 0.925:
						tmpy.append({'type':1, 'value':"V"})
					else:
						tmpy.append({'type':2, 'value':"L"})

				else:
					if x < 0.2:
						tmpy.append({'type':0, 'value':0})
					elif x < 0.4:
						tmpy.append({'type':1, 'value':int((x-0.2)*500)})
					elif x < 0.6:
						tmpy.append({'type':2, 'value':int((x-0.4)*500)})
					else:
						if (x-0.6)*20 < 6:
							tmpy.append({'type': int((x-0.6)*20+4), 'value':0})
						else:
							tmpy.append({'type':0, 'value':0})
			tmpz.append(tmpy[:])
		res.append(tmpz[:])
	return res

def get_randomized_matrix(b, ans=False, seed=None):
	random.seed(seed)
	mat = get_matrix(b, ans)
	vias = {}
	lines = {}
	for z, zv in enumerate(mat):
		for y, yv in enumerate(zv):
			for x, xv in enumerate(yv):
				if not ans:
					if 0.2 <= xv and xv < 0.4:
						if not xv in lines:
							lines[xv] = []
						lines[xv].append((x, y, z))
					elif 0.4 <= xv and xv <= 0.6:
						if not xv in vias:
							vias[xv] = []
						vias[xv].append((x, y, z))

	if not ans:
		for i in range(100):
			s = random.sample(lines.keys(), 2)
			tmp = lines[s[0]][:]
			lines[s[0]] = lines[s[1]][:]
			lines[s[1]] = tmp[:]

			s = random.sample(vias.keys(), 2)
			tmp = vias[s[0]][:]
			vias[s[0]] = vias[s[1]][:]
			vias[s[1]] = tmp[:]

		for k, v in lines.items():
			for v2 in v:
				mat[v2[2]][v2[1]][v2[0]] = k

		for k, v in vias.items():
			for v2 in v:
				mat[v2[2]][v2[1]][v2[0]] = k

	return mat

def main():
	pass

if __name__ == '__main__':
	# main()

	# boards = read_problem("../../ex_q.txt")
	boards = read_problem("../../NL_Q06.txt")
	# boards2 = read_answer("../../ex_a.txt", problem = boards)
	boards2 = read_answer("../../T01_A06.txt", problem = boards)

	print_boards(boards)
	# print_boards(boards2)

	res = get_matrix(boards2, ans=True)

	for z in res:
		for y in z:
			for x in y:
				if x < 0 : x = 0
				sys.stdout.write("{0:3.2f} ".format(x))
			sys.stdout.write("\n")
		sys.stdout.write("\n")


	res = get_randomized_matrix(boards2, ans=True)

	for z in res:
		for y in z:
			for x in y:
				if x < 0 : x = 0
				sys.stdout.write("{0:3.2f} ".format(x))
			sys.stdout.write("\n")
		sys.stdout.write("\n")
