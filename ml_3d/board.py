# -*- encoding: utf-8 -*-

class Board:

	def __init__(self, board, n_dims):
		self.board = board
		self.lines = {}
		self.vias = {}
		self.via_to_line = {}
		self.via_internal_id = {}
		self.n_dims = n_dims
		self.n_dims_half = n_dims / 2

		ndh = self.n_dims_half

		for z, zl in enumerate(board):
			for y, yl in enumerate(zl):
				for x, xl in enumerate(yl):
					_key = xl["data"]
					if xl["type"] == 1:
						if not _key in self.lines:
							self.lines[_key] = []
						self.lines[_key].append((x-ndh, y-ndh, z+1))
					elif xl["type"] == 'via':
						if not _key in self.vias:
							self.vias[_key] = []
						self.vias[_key].append((x-ndh, y-ndh, z+1))
						if not _key in self.via_internal_id:
							self.via_internal_id[_key] = xl["shape"]

	def __call__(self):
		return self.board

	"""
	x, y, zはNoneかintかlambda式
	"""
	def search_line(self, name, x=None, y=None, z=None):

		assert(x is None or isinstance(x, int) or hasattr(x, '__call__'))
		assert(y is None or isinstance(y, int) or hasattr(y, '__call__'))
		assert(z is None or isinstance(z, int) or hasattr(z, '__call__'))

		if isinstance(x, int): tx = lambda n: x == n
		else: tx = x

		if isinstance(y, int): ty = lambda n: y == n
		else: ty = y

		if isinstance(z, int): tz = lambda n: z == n
		else: tz = z

		res = None
		if not name in self.lines:
			return res

		line = self.lines[name]

		if tx is None and ty is None and tz is None:
			res = line
		else:
			for _l in line:
				if tx is None: pass
				elif tx(_l[0]): pass
				else: continue

				if ty is None: pass
				elif ty(_l[1]): pass
				else: continue

				if tz is None: pass
				elif tz(_l[2]): pass
				else: continue

				res = _l

		return res


	"""
	x, y, zはNoneかintかlambda式
	"""
	def search_via(self, x=None, y=None, z=None):

		assert(x is None or isinstance(x, int) or hasattr(x, '__call__'))
		assert(y is None or isinstance(y, int) or hasattr(y, '__call__'))
		assert(z is None or isinstance(z, int) or hasattr(z, '__call__'))

		if isinstance(x, int): tx = lambda n: x == n
		else: tx = x

		if isinstance(y, int): ty = lambda n: y == n
		else: ty = y

		if isinstance(z, int): tz = lambda n: z == n
		else: tz = z

		res = None

		vias = []

		for k, v in self.vias.items():
			vias += [(k, v2) for v2 in v]

		if tx is None and ty is None and tz is None:
			res = vias
		else:
			res = []
			for _l in vias:
				if tx is None: pass
				elif tx(_l[1][0]): pass
				else: continue

				if ty is None: pass
				elif ty(_l[1][1]): pass
				else: continue

				if tz is None: pass
				elif tz(_l[1][2]): pass
				else: continue

				res.append(_l)

		return res

	"""
	反対側のviaを返す
	"""
	def get_opposite_via(self, name, z):

		if not name in self.vias:
			return None

		vias = self.vias[name]

		maxz = max([v[2] for v in vias])
		minz = min([v[2] for v in vias])

		tvia = self.vias[name][0]

		res = None

		if z == maxz:
			res = minz
		elif z == minz:
			res = maxz

		if not res is None:
			return self.search_via(x=tvia[1][0], y=tvia[1][1], z=res)
		else:
			return None

	def get_vias_key(self):
		return self.vias.keys()
	def get_lines_key(self):
		return self.lines.keys()
	def get_via_internal_id(self, _key):
		return self.via_internal_id[_key]

	"""
	周囲+/-distマスの状況を返す．
	結果を参照するときは，中心からの相対的な位置で参照する．
	例）セルx, yに対し，+1, +1を参照するとき：res[1][1]
	例）セルx, yに対し，-1, -1を参照するとき：res[-1][-1]
	"""
	def get_sorroundings(self, dist, x, y=None, z=None):

		if isinstance(x, int):
			_x = x
			_y = y
			_z = z-1
		elif isinstance(x, tuple):
			_x = x[0]
			_y = x[1]
			_z = x[2]-1

		_y_len = len(self.board[0])
		_x_len = len(self.board[0][0])

		b = self.board[_z]
		res = [[-1.0 for i in range(0, dist*2+1)] for j in range(0, dist*2+1)]

		ndh = self.n_dims_half

		for ty in range(max(ndh, _y-dist+ndh), min(_y+dist+1+ndh, _y_len-ndh)):
			for tx in range(max(ndh, _x-dist+ndh), min(_x+dist+1+ndh, _x_len-ndh)):
				if b[ty][tx]['data'] == -1:
					cellx = -1.0
				elif b[ty][tx]['type'] == 1:
					cellx = 0.5
				elif b[ty][tx]['type'] == 'via':
					if self.is_middle_via(b[ty][tx]['data'], _z+1):
						cellx = -1.0
					else:
						cellx = 1.0
				else:
					cellx = 0.0
				res[ty-ndh-_y][tx-ndh-_x] = cellx

		return res

	"""
	ビア名とz座標を与えると，そのビアの座標が中間かどうかを判定してくれる．
	"""
	def is_middle_via(self, name, z):

		if not name in self.vias:
			return None

		vias = self.vias[name]

		maxz = max([v[2] for v in vias])
		minz = min([v[2] for v in vias])

		if (z != minz) and (z != maxz):
			return True
		else:
			return False

	def set_via_to_line(self, via, line):

		if line is None:
			self.via_to_line.pop(via)
		else:
			self.via_to_line[via] = line

	"""
	単層形式で出力
	"""
	def output_boards(self, fpath="./problem", mode="single"):

		assert(mode in ['single', 'multi'])

		lines = [{} for z in range(0, len(self.board))]

		z_num = len(self.board)
		y_num = len(self.board[0]) - self.n_dims_half*2
		x_num = len(self.board[0][0]) - self.n_dims_half*2

		if mode == 'single':
			for z in range(0, len(self.board)):
				for k, _lines in self.lines.items():
					for _line in _lines:
						if _line[2] == z+1:
							if k in lines[z]:
								lines[z][k].append(_line)
							else:
								lines[z][k] = [_line]

				for k, _vias in self.vias.items():
					for _via in _vias:
						if _via[2] == z+1:
							_key = self.via_to_line[k]
							if _key in lines[z]:
								lines[z][_key].append(_via)
							else:
								lines[z][_key] = [_via]

			for z, zv in enumerate(lines):
				stxt = []
				stxt.append("SIZE {}X{}\n".format(x_num, y_num))
				stxt.append("LINE_NUM {}\n".format(len(zv)))
				for k, v in zv.items():
					print k, v
					stxt.append("LINE#{0} ({1[0][0]},{1[0][1]})-({1[1][0]},{1[1][1]})\n".format(k, v))

				fn = "{}_L{}.txt".format(fpath, z+1)
				fp = open(fn, "w")
				fp.writelines(stxt)
				fp.close()

		elif mode == 'multi':
			stxt = []
			stxt.append("SIZE {}X{}X{}".format(x_num, y_num, z_num))
			stxt.append("LINE_NUM {}".format(len(self.lines)))
			for k, _lines in self.lines.items():
				txt = "LINE#{0} ({1[0][0]},{1[0][1]},{1[0][2]}) ({1[1][0]},{1[1][1]},{1[1][2]})".format(k, _lines)
				for k2, v2 in self.via_to_line.items():
					if v2 == k:
						_line_to_via = self.get_via_internal_id(k2)
						txt = "{} {}".format(txt, _line_to_via)
				stxt.append(txt)
			for k, _vias in self.vias.items():
				txt = "VIA#{}".format(k)
				for v in _vias:
					txt += " ({0[0]},{0[1]},{0[2]})".format(v)
				stxt.append(txt)
			stxt = map(lambda x: x+"\n", stxt)

			fn = "{}_{}.txt".format(fpath, "3D")
			fp = open(fn, 'w')
			fp.writelines(stxt)
			fp.close()



	@staticmethod
	def mdist(l1, l2):
		# return abs(l1[0] - l2[0]) + abs(l1[1] - l2[1])
		return max(abs(l1[0] - l2[0]), abs(l1[1] - l2[1]))*2
