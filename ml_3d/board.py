# -*- encoding: utf-8 -*-

class Board:

	def __init__(self, board):
		self.board = board
		self.lines = {}
		self.vias = {}

		for z, zl in enumerate(board):
			for y, yl in enumerate(zl):
				for x, xl in enumerate(yl):
					_key = xl["data"]
					if xl["type"] == 1:
						if not _key in self.lines:
							self.lines[_key] = []
						self.lines[_key].append((x-1, y-1, z+1))
					elif xl["type"] == 'via':
						if not _key in self.vias:
							self.vias[_key] = []
						self.vias[_key].append((x-1, y-1, z+1))

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

		for v in self.vias.values():
			vias += v[:]

		if tx is None and ty is None and tz is None:
			res = vias
		else:
			res = []
			for _l in vias:
				if tx is None: pass
				elif tx(_l[0]): pass
				else: continue

				if ty is None: pass
				elif ty(_l[1]): pass
				else: continue

				if tz is None: pass
				elif tz(_l[2]): pass
				else: continue

				res.append(_l)

		return res
