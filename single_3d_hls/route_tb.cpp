/**
 * route_tb.cpp
 *
 * for Vivado HLS
 */

#ifndef SOFTWARE
#include <stdio.h>
#include <string.h>
#include <ap_int.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "main.hpp"
#include "route.hpp"

//Board* board; // 対象ボード

//int penalty_T; // penalty of "touch"
//int penalty_C; // penalty of "cross"
//int penalty_V; // penalty of "via duplication"

//bool routing(ap_uint<7> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, Board *board, ap_int<4> *output);

#if 0
/**
 * 問題盤の初期化 (テスト用)
 */
void initialize_test(Board *board, int size_x, int size_y, int size_z){

	{
		Box* trgt_box_0 = board->box(0, 0, 0);
		Box* trgt_box_1 = board->box(4, 4, 1);
		trgt_box_0->setTypeNumber();
		trgt_box_1->setTypeNumber();
		trgt_box_0->setIndex(1);
		trgt_box_1->setIndex(1);
		Line* trgt_line = board->line(1);
		trgt_line->setSourcePort(0, 0, 0);
		trgt_line->setSinkPort(4, 4, 1);
		trgt_line->setHasLine(true);
	}

	{
		Box* trgt_box_0 = board->box(2, 2, 0);
		Box* trgt_box_1 = board->box(2, 2, 1);
		trgt_box_0->setTypeVia();
		trgt_box_1->setTypeVia();
		trgt_box_0->setIndex(1);
		trgt_box_1->setIndex(1);
		Via* trgt_via = board->via(1);
		trgt_via->setSourcePort(2, 2, 0);
		trgt_via->setSinkPort(2, 2, 1);
	}

	for(int z=0;z<size_z;z++){
		for(int y=0;y<size_y;y++){
			for(int x=0;x<size_x;x++){
				Box* trgt_box = board->box(x,y,z);
				if(!(trgt_box->isTypeNumber() || trgt_box->isTypeVia() || trgt_box->isTypeInterVia()))
					trgt_box->setTypeBlank();
			}
		}
	}

}

void printSolution(Board *board){
	int for_print[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				for_print[z][y][x] = -1;
			}
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		if(!trgt_line->getHasLine()){
			for_print[trgt_line->getSourceZ()][trgt_line->getSourceY()][trgt_line->getSourceX()] = i;
			for_print[trgt_line->getSinkZ()][trgt_line->getSinkY()][trgt_line->getSinkX()] = i;
			continue;
		}
		Point *trgt_track = trgt_line->track;
		for(int j=0;j<trgt_line->track_index;j++){
			Point *p = trgt_track + j;
			for_print[p->z][p->y][p->x] = i;
		}
	}
	for(int i=1;i<=board->getViaNum();i++){
		Via* trgt_via = board->via(i);
		int via_x = trgt_via->getSourceX();
		int via_y = trgt_via->getSourceY();
		int via_z = trgt_via->getSourceZ();
		int line_num = for_print[via_z][via_y][via_x]; 
		for(int z=via_z+1;z<trgt_via->getSinkZ();z++){
			for_print[z][via_y][via_x] = line_num;
		}
	}
	
	cout << endl;
	cout << "SOLUTION" << endl;
	cout << "========" << endl;

	for(int z=0;z<board->getSizeZ();z++){
		cout << "LAYER " << z+1 << endl;
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				int n = for_print[z][y][x];
				if(n < 0){
					// 線が引かれていないマス："00"表示
					cout << " 00";
				}else{
					Box* trgt_box = board->box(x,y,z);
					if(trgt_box->isTypeNumber()){
						// 線が引かれているマス [端点] (2桁表示)
						cout << " " << setfill('0') << setw(2) << n;
					}else{
						// 線が引かれているマス [非端点] (2桁表示)
						cout << " " << setfill('0') << setw(2) << n;
					}
				}
			}
			cout << endl;
		}
		cout << endl;
	}
}

void recordLine(int trgt_line_id, Board *board){

	Line* trgt_line = board->line(trgt_line_id);
	Point *trgt_track = trgt_line->track;

	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int old_z = trgt_line->getSinkZ();
	int new_x = (trgt_track + 1)->x;
	int new_y = (trgt_track + 1)->y;
	int new_z = (trgt_track + 1)->z;
	Box* old_box = board->box(old_x,old_y,old_z);
	Box* new_box = board->box(new_x,new_y,new_z);

	if(new_box->isTypeVia());
	else if(new_x==old_x && new_y==old_y-1){ // 北
		new_box->incrementSouthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		new_box->incrementWestNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		new_box->incrementNorthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		new_box->incrementEastNum();
	}

	old_x = new_x; old_y = new_y; old_z = new_z;
	old_box = new_box;

	for(int i=2;i < (trgt_line->track_index) -1 ;i++){
		new_x = (trgt_track + i)->x;
		new_y = (trgt_track + i)->y;
		new_z = (trgt_track + i)->z;
		new_box = board->box(new_x,new_y,new_z);

		if(new_box->isTypeVia() && old_box->isTypeVia()){
			board->via(old_box->getIndex())->incrementUsedLineNum();
		}
		else{
			if(new_x==old_x && new_y==old_y-1){ // 北
				if(old_box->isTypeBlank()) old_box->incrementNorthNum();
				if(new_box->isTypeBlank()) new_box->incrementSouthNum();
			}
			else if(new_x==old_x+1 && new_y==old_y){ // 東
				if(old_box->isTypeBlank()) old_box->incrementEastNum();
				if(new_box->isTypeBlank()) new_box->incrementWestNum();
			}
			else if(new_x==old_x && new_y==old_y+1){ // 南
				if(old_box->isTypeBlank()) old_box->incrementSouthNum();
				if(new_box->isTypeBlank()) new_box->incrementNorthNum();
			}
			else if(new_x==old_x-1 && new_y==old_y){ // 西
				if(old_box->isTypeBlank()) old_box->incrementWestNum();
				if(new_box->isTypeBlank()) new_box->incrementEastNum();
			}
		}

		old_x = new_x; old_y = new_y; old_z = new_z;
		old_box = new_box;
	}

	new_x = trgt_line->getSourceX();
	new_y = trgt_line->getSourceY();
	new_z = trgt_line->getSourceZ();

	if(old_box->isTypeVia());
	else if(new_x==old_x && new_y==old_y-1){ // 北
		old_box->incrementNorthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		old_box->incrementEastNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		old_box->incrementSouthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		old_box->incrementWestNum();
	}
}
#endif

int main() {
	using namespace std;

#if 0
	int size_x = 5, size_y = 6, size_z = 2;
	int line_num = 1;
	int via_num = 1;

	Board *board = new Board(size_x, size_y, size_z, line_num, via_num);
	initialize_test(board, size_x, size_y, size_z);
#endif

	//ap_int<8> status;
	ap_int<8> rawboard[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	bool result = routing(1, 0, 0, 0, /*board, &status*/ rawboard);
	if (result)
		cout << "Test Passed!" << endl;
	else
		cout << "Test Failed!" << endl;
	//cout << "status = " << status << endl;

#if 0
	printSolution(board);
#endif

	return 0;
}
