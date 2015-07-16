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
/** グローバル変数定義 **/
/*******************************************************/

Board* board; // 対象ボード

int penalty_T; // penalty of "touch"
int penalty_C; // penalty of "cross"

vector<int> adj_num; // 固定線に隣接する数字を格納

//map<int,Cost> cost_table; // コスト表
//int calc_T; // 計算するとき用
//int calc_C; // 計算するとき用


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


// ２桁の整数をアルファベットに変換（３５以下）
char changeIntToChar(int n){
	
	switch(n){
		case 10: return 'A'; break;
		case 11: return 'B'; break;
		case 12: return 'C'; break;
		case 13: return 'D'; break;
		case 14: return 'E'; break;
		case 15: return 'F'; break;
		case 16: return 'G'; break;
		case 17: return 'H'; break;
		case 18: return 'I'; break;
		case 19: return 'J'; break;
		case 20: return 'K'; break;
		case 21: return 'L'; break;
		case 22: return 'M'; break;
		case 23: return 'N'; break;
		case 24: return 'O'; break;
		case 25: return 'P'; break;
		case 26: return 'Q'; break;
		case 27: return 'R'; break;
		case 28: return 'S'; break;
		case 29: return 'T'; break;
		case 30: return 'U'; break;
		case 31: return 'V'; break;
		case 32: return 'W'; break;
		case 33: return 'X'; break;
		case 34: return 'Y'; break;
		case 35: return 'Z'; break;
	}
}

// 問題盤を表示
void printBoard(){

	cout << "*** BOARD ***" << endl;
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				cout << "+ ";
			}
			else{
				int trgt_num = trgt_box->getNumber();
				if(trgt_num < 10){
					cout << trgt_num << " ";
				}
				else{
					// アルファベット表示
					cout << changeIntToChar(trgt_num) << " ";
				}
			}
		}
		cout << endl;
	}
}

// 現状のラインを表示
void printLine(int i){

	cout << endl;
	cout << "print Line" << i << endl;
	Line* trgt_line = board->line(i);
	vector<int>* trgt_track = trgt_line->getTrack();
	
	map<int,map<int,int> > for_print;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				for_print[y][x] = -1;
			}
			else{
				for_print[y][x] = trgt_box->getNumber();
			}
		}
	}
	for(int j=0;j<(int)(trgt_track->size());j++){
		int point = (*trgt_track)[j];
		int point_x = point % board->getSizeX();
		int point_y = point / board->getSizeX();
		for_print[point_y][point_x] = -2;
	}
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] == -2) cout << "@";
			else if(for_print[y][x] == -1) cout << "+";
			else{
				if(for_print[y][x] < 10){
					cout << for_print[y][x];
				}
				else{
					cout << changeIntToChar(for_print[y][x]);
				}
			}
		}
		cout << endl;
	}
}

// 正解を表示（正解表示は数字２桁）
void printSolution(){

	map<int,map<int,int> > for_print;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				for_print[y][x] = -1;
			}
			else{
				for_print[y][x] = trgt_box->getNumber();
			}
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		vector<int>* trgt_track = trgt_line->getTrack();
		for(int j=0;j<(int)(trgt_track->size());j++){
			int point = (*trgt_track)[j];
			int point_x = point % board->getSizeX();
			int point_y = point / board->getSizeX();
			for_print[point_y][point_x] = i;
		}
	}
	
	cout << "*** SOLUTION ***" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] < 0){
				// 線が引かれていないマス："00"表示
				cout << "00";
			}
			else{
				// その他(2桁表示)
				if(for_print[y][x] < 10){
					cout << "0" << for_print[y][x];
				}
				else
					cout << for_print[y][x];
			}
			if(x!=(board->getSizeX()-1)){
				cout << ",";
			}
		}
		cout << endl;
	}
}


#endif /*_MAIN_HPP_*/
