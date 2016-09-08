/**
 * main.hpp
 *
 * for Vivado HLS
 */

#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

//#include <map>
//#include <vector>
//#include <queue>

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

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

// 各種設定値
//#define MAX_BOXES 40
#define MAX_BOXES 20
//#define MAX_LAYER 8
#define MAX_LAYER 2
#define MAX_LINES 100
//#define MAX_TRACKS 160
#define MAX_TRACKS 80
#define MAX_SEARCH 1000


struct Point{
	ap_int<7> x;
	ap_int<7> y;
	ap_int<5> z;
};

#include "board.hpp"
#include "box.hpp"
#include "line.hpp"
#include "via.hpp"

struct Search{
	ap_int<7> x;
	ap_int<7> y;
	ap_int<8> d; // 方向
};

struct Direction{
	// 方向
	bool n;	// 北
	bool s;	// 南
	bool e;	// 東
	bool w;	// 西
};

struct IntraBox_4{
	ap_int<16> ne;
	ap_int<16> nw;
	ap_int<16> se;
	ap_int<16> sw;
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
	ap_int<8> c_n;
	ap_int<8> c_s;
	ap_int<8> c_e;
	ap_int<8> c_w;
};

struct IntraBox_1{
	ap_int<16> cost;
	Direction_R d;
};

#endif /*_MAIN_HPP_*/
