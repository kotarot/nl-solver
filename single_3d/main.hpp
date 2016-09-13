#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <map>
#include <vector>
#include <queue>

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define NOT_USE -1

// Inter-Box
#define NORTH 1
#define EAST  2
#define SOUTH 3
#define WEST  4

// Intra-Box
#define NE 1
#define NW 2
#define SE 3
#define SW 4

// Touch & Cross
#define NT 1  // ループ毎にペナルティの更新するときの定数 (タッチ)
#define NC 2  // ループ毎にペナルティの更新するときの定数 (クロス)
#define NV 2  // ループ毎にペナルティの更新するときの定数 (ビア重複)
#define ML 1  // コストの計算（配線長）
#define BT 1  // コストの計算（曲がり回数）

#define O_LOOP  500 // 外ループ回数
#define I_LOOP 2000 // 内ループ回数

struct Point{
	int x;
	int y;
	int z;
};

#include "box.hpp"
#include "line.hpp"
#include "via.hpp"
#include "board.hpp"

struct Search{
	int x;
	int y;
	int d; // 方向
};

struct Direction{
	// 方向
	bool n;	// 北
	bool s;	// 南
	bool e;	// 東
	bool w;	// 西
};

struct IntraBox_4{
	int ne;
	int nw;
	int se;
	int sw;
	Direction d_ne;
	Direction d_nw;
	Direction d_se;
	Direction d_sw;
};

struct Direction_R{
	// 方向
	bool n;	// 北
	bool s;	// 南
	bool e;	// 東
	bool w;	// 西
	// ひとつ前の方向
	int c_n;
	int c_s;
	int c_e;
	int c_w;
};

struct IntraBox_1{
	int cost;
	Direction_R d;
};


/*******************************************************/
/** グローバル関数定義 **/
/*******************************************************/

void initialize(char* filename);
bool isFinished();

#endif /*_MAIN_HPP_*/
