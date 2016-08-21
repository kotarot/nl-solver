# -*- encoding: utf-8 -*-

class Board:

	junctions = ['', '', '', '', '┘', '─', '└', '┐', '│', '┌']

	def __init__(self):
		self.x = 0
		self.y = 0
		self.z = 0
		self.lines = []
		self.vias = []
		self.boards = [[[]]]
		self.values = [[[]]]
		self.types = [[[]]]

	def __call__(self):
		pass

	def print_board(self):
		lcnt = 1
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


