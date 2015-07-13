#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <sstream>
#include <math.h>
#include <iomanip>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

#define P_MAX 4 // 複製プロセス数

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
/** クラス定義 **/
/*******************************************************/

// ボックス（マス）クラス
class Box{
public:
	enum BoxType {TYPE_NULL,TYPE_NUMBER,TYPE_BLANK};
	enum FixType {ALL_FIXED,HALF_FIXED,NO_FIXED};
	static const int NOT_USE = -1;
	
	Box(int _x,int _y):type(TYPE_NULL),f_type(NO_FIXED),num(NOT_USE),north(0),south(0),east(0),west(0),f_north(false),f_south(false),f_east(false),f_west(false),self_count(0),candidate(false){x=_x;y=_y;}
	~Box();
	
	bool isTypeNumber() const{return (type==TYPE_NUMBER);}
	void setTypeNumber(){type=TYPE_NUMBER;}
	
	bool isTypeBlank() const{return (type==TYPE_BLANK);}
	void setTypeBlank(){type=TYPE_BLANK;}
	
	bool isTypeAllFixed() const{return (f_type==ALL_FIXED);}
	void setTypeAllFixed(){f_type=ALL_FIXED;}
	
	bool isTypeHalfFixed() const{return (f_type==HALF_FIXED);}
	void setTypeHalfFixed(){f_type=HALF_FIXED;}
	
	int getNumber(){return num;}
	void setNumber(int _num){num=_num;}
	
	int getX(){return x;}
	int getY(){return y;}
	
	void incrementNorthNum(){north++;}
	void decrementNorthNum(){north--;}
	int getNorthNum(){return north;}
	
	void incrementEastNum(){east++;}
	void decrementEastNum(){east--;}
	int getEastNum(){return east;}
	
	void incrementSouthNum(){south++;}
	void decrementSouthNum(){south--;}
	int getSouthNum(){return south;}
	
	void incrementWestNum(){west++;}
	void decrementWestNum(){west--;}
	int getWestNum(){return west;}
	
	void fixNorthLine(){f_north=true;}
	void fixEastLine(){f_east=true;}
	void fixSouthLine(){f_south=true;}
	void fixWestLine(){f_west=true;}
	
	bool isNorthLineFixed(){return f_north;}
	bool isEastLineFixed(){return f_east;}
	bool isSouthLineFixed(){return f_south;}
	bool isWestLineFixed(){return f_west;}
	
	void incrementSelfCount(){self_count++;}
	void resetSelfCount(){self_count=0;}
	int getSelfCount(){return self_count;}
	
	void setCandidate(){candidate=true;}
	void setNonCandidate(){candidate=false;}
	bool isCandidate(){return candidate;}
	

private:
	BoxType type;	// 数字マスか空白マス
	FixType f_type; // 固定マス
	int num;		// 数字マスの数字
	int x, y;		// 座標
	int north;		// 北向きに存在する線の数
	int south;		// 南向きに存在する線の数
	int east;		// 東向きに存在する線の数
	int west;		// 西向きに存在する線の数
	bool f_north;	// 北向きに固定線を持つ？
	bool f_south;	// 南向きに固定線を持つ？
	bool f_east;	// 東向きに固定線を持つ？
	bool f_west;	// 西向きに固定線を持つ？
	
	int self_count; // ライン不通過回数カウント用変数
	bool candidate; // 中間ポートの候補？
};

// ラインクラス
class Line{
public:
	static const int NOT_USE = -1;

	Line(int _index):x_0(NOT_USE),y_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),x_i(NOT_USE),y_i(NOT_USE),candidate(false),im(false){index=_index;track.clear();}
	~Line();

	void setSourcePort(int x,int y){x_0=x;y_0=y;}
	int getSourceX(){return x_0;}
	int getSourceY(){return y_0;}
	
	void setSinkPort(int x,int y){x_1=x;y_1=y;}
	int getSinkX(){return x_1;}
	int getSinkY(){return y_1;}
	
	// 中間ポート
	void setIntermediatePort(int x,int y){x_i=x;y_i=y;}
	int getInterX(){return x_i;}
	int getInterY(){return y_i;}
	
	void setCandidate(){candidate=true;}
	void setNonCandidate(){candidate=false;}
	bool isCandidate(){return candidate;}
	
	void setIntermediateUse(){im=true;}
	void setIntermediateUnuse(){im=false;}
	bool isIntermediateUsed(){return im;}
	
	void changePort(){	// ソースとシンクの交換
		int tmp_x = x_0;
		x_0 = x_1;
		x_1 = tmp_x;
		int tmp_y = y_0;
		y_0 = y_1;
		y_1 = tmp_y;
	}
	
	int getLineLength(){return track.size();}
	
	vector<int>* getTrack(){return &track;}
	void pushIntToTrack(int t){track.push_back(t);}
	void clearTrack(){track.clear();}

private:
	int index;		// ライン番号
	int x_0, y_0;	// ソースポート
	int x_1, y_1;	// シンクポート
	int x_i, y_i;	// 中間ポート
	bool candidate; // 中間ポートを利用する候補か？
	bool im;		// 中間ポートを利用するか？
	vector<int> track;
};

// ボードクラス
class Board{
public:
	Board(int _x,int _y,int _ln){
		size_x = _x;
		size_y = _y;
		line_num = _ln;
		for(int y=0;y<size_y;y++){
			for(int x=0;x<size_x;x++){
				Box* box;
				box = new Box(x,y);
				boxes[y][x] = box;
			}
		}
		for(int i=1;i<=line_num;i++){
			Line* line;
			line = new Line(i);
			lines[i] = line;
		}
	}
	~Board(){
		// デストラクタ（要メモリ解放）
	}
	
	int getSizeX(){return size_x;}
	int getSizeY(){return size_y;}
	int getLineNum(){return line_num;}
	
	Box* box(int x,int y){return boxes[y][x];}	// マスの取得
	Line* line(int idx){return lines[idx];}		// ラインの取得
	
private:
	int size_x;		// X方向サイズ
	int size_y;		// Y方向サイズ
	int line_num;	// ライン数
	map<int,map<int,Box*> > boxes;	// マスの集合
	map<int,Line*> lines;	// ラインの集合
};


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

bool proc(int idx);
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
void printSolution(int idx){

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
	
	ostringstream os;
	os << idx;
	string filename = "answer_" + os.str() + ".txt";
	ofstream ofs(filename.c_str());
	
	//cout << "*** SOLUTION ***" << endl;
	ofs << "SIZE " << board->getSizeX() << "X" << board->getSizeY() << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] < 0){
				// 線が引かれていないマス："00"表示
				ofs << "00";
			}
			else{
				// その他(2桁表示)
				if(for_print[y][x] < 10){
					ofs << "0" << for_print[y][x];
				}
				else
					ofs << for_print[y][x];
			}
			if(x!=(board->getSizeX()-1)){
				ofs << ",";
			}
		}
		ofs << endl;
	}
}


#endif /*_MAIN_HPP_*/
