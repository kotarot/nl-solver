#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <map>
#include <vector>
#include <queue>

#include <assert.h>
#include <getopt.h>
#include <stdlib.h>

using namespace std;

struct Point{
	int x;
	int y;
};

#include "../single/box.hpp"
#include "../single/line.hpp"
#include "../single/board.hpp"

#include "utils.hpp"


void initialize(char* filename);

#endif /* __MAIN_HPP__ */
