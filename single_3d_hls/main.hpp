/**
 * main.hpp
 *
 * for Vivado HLS
 */

#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

using namespace std;

#define NOT_USE -1

// Inter-Box
#define NORTH 0
#define EAST  1
#define SOUTH 2
#define WEST  3

// Intra-Box
#define NE 0
#define NW 1
#define SE 2
#define SW 3

// Touch & Cross
#define NT 1  // ループ毎にペナルティの更新するときの定数 (タッチ)
#define NC 2  // ループ毎にペナルティの更新するときの定数 (クロス)
#define NV 2  // ループ毎にペナルティの更新するときの定数 (ビア重複)
#define ML 1  // コストの計算（配線長）
#define BT 1  // コストの計算（曲がり回数）

#define O_LOOP  500 // 外ループ回数
#define I_LOOP 2000 // 内ループ回数

// 各種設定値
#define MAX_BOXES 40
#define MAX_LAYER 8
#define MAX_LINES 100
#define MAX_VIAS  100
#define MAX_TRACKS 160
#define MAX_SEARCH 8000

#define BOARDSTR_SIZE 12800 // = 8 * 40 * 40

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


// メルセンヌ・ツイスタ
void mt_init_genrand(unsigned long s);
unsigned long mt_genrand_int32(int a, int b);

bool nlsolver(char boardstr[BOARDSTR_SIZE], ap_int<8> *status);
void initialize(char boardstr[BOARDSTR_SIZE], Board *board);
bool isFinished(Board *board);
void generateSolution(char boardstr[BOARDSTR_SIZE], Board *board);


#endif /*_MAIN_HPP_*/
