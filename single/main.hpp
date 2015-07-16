#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <limits.h>

using namespace std;

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
#define ML 1
#define NT 1
#define NC 2
#define BT 1

#define O_LOOP  200 // 外ループ回数
#define INIT     30 // 不通過マスをチェックし始める(外ループの)回数
#define I_LOOP 2000 // 内ループ回数
#define LIMIT  1800 // 何回以上で不通過マスと判定するか(内ループ)

#include "box.hpp"
#include "line.hpp"
#include "board.hpp"

#include "utils.hpp"


struct Search{
	int x;
	int y;
	int d; // 方向
	int c; // ひとつ前の方向
};

struct Direction{
	bool n; // 北
	bool s; // 南
	bool e; // 東
	bool w; // 西
};

struct IntraBox{
	int ne;
	int nw;
	int se;
	int sw;
	Direction d_ne;
	Direction d_nw;
	Direction d_se;
	Direction d_sw;
};

//struct Cost{
//	int mL; // 長さ
//	int nT;	// タッチ数
//	int nC; // クロス数
//};


/*******************************************************/
/** グローバル関数定義 **/
/*******************************************************/

void initialize(char* filename);
void generateFixFlag();
int getConnectedNumber(int x,int y);
void setAdjacentNumberProc(int x,int y);
void setAdjacentNumber(int x,int y,int d_1,int d_2);
bool routing(int trgt_line_id);
bool isInserted(int x,int y,int d);
bool isFixed(int x,int y,int d,int c,int num);
int countLineNum(int x,int y);
void recording(int trgt_line_id);
void deleting(int trgt_line_id);
bool isFinished();
void resetCandidate();
void checkLineNonPassed();
bool routingSourceToI(int trgt_line_id);
bool routingIToSink(int trgt_line_id);
void checkCandidateLine(int x,int y);


#endif /*_MAIN_HPP_*/
