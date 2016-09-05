/**
 * route.cpp
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

#include "./main.hpp"
#include "./route.hpp"
//#include "./utils.hpp"


// ================================ //
// メルセンヌ・ツイスタ
// ================================ //
#include "mt19937ar.hpp"

#if 0
void mt_init_genrand(unsigned long s) {
	init_genrand(s);
}
#endif

// AからBの範囲の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
	return genrand_int32() % (b - a + 1) + a;
}


//Board* board; // 対象ボード

//int penalty_T; // penalty of "touch"
//int penalty_C; // penalty of "cross"
//int penalty_V; // penalty of "via duplication"

bool routing(ap_uint<7> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, Board *board, ap_int<4> *output){
#pragma HLS INTERFACE s_axilite port=trgt_line_id bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=penalty_T bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=penalty_C bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=penalty_V bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=board bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=output bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->track_index = 0;

	// ボードの初期化
	IntraBox_4 my_board_1[MAX_BOXES][MAX_BOXES]; // ソース側のボード
	IntraBox_4 my_board_2[MAX_BOXES][MAX_BOXES]; // シンク側のボード
	IntraBox_4 init = {
		INT_MAX,INT_MAX,INT_MAX,INT_MAX,
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false}};
	for (int y = 0; y < board->getSizeY(); y++) {
		for (int x = 0; x < board->getSizeX(); x++) {
			my_board_1[y][x] = init;
			my_board_2[y][x] = init;
		}
	}
	int start_x, start_y;
	IntraBox_4* start;
	//queue<Search> qu;
	Search qu[MAX_SEARCH];
	int qu_head = 0;
	int qu_tail = 0;

	int start_z = trgt_line->getSourceZ();
	int end_z = trgt_line->getSinkZ();


	/*** ソース層の探索 ***/

	// スタート地点の設定
	start_x = trgt_line->getSourceX();
	start_y = trgt_line->getSourceY();
	start = &(my_board_1[start_y][start_x]);
	start->ne = 0; start->nw = 0;
	start->se = 0; start->sw = 0;

	// 北方向を探索
	if(isInserted_1(start_x,start_y-1,start_z, board)){
		Search trgt = {start_x,start_y-1,SOUTH};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 東方向を探索
	if(isInserted_1(start_x+1,start_y,start_z, board)){
		Search trgt = {start_x+1,start_y,WEST};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 南方向を探索
	if(isInserted_1(start_x,start_y+1,start_z, board)){
		Search trgt = {start_x,start_y+1,NORTH};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 西方向を探索
	if(isInserted_1(start_x-1,start_y,start_z, board)){
		Search trgt = {start_x-1,start_y,EAST};
		qu[qu_tail] = trgt; qu_tail++;
	}

	while (qu_head != qu_tail) {

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,start_z);
		IntraBox_4* trgt_ibox = &(my_board_1[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_4* find_ibox = &(my_board_1[trgt.y+1][trgt.x]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getSouthNum();
			if(touch_count < 0){ *output = 10; return false; /*cout << "error! (error: 10)" << endl; exit(10);*/ }
			// コスト
			int cost_se = (find_ibox->ne) + ML + touch_count * penalty_T;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			int cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = true;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).s = true;
			}
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = true;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).s = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = true;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).s = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = true;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).s = true;
			}
		}
		if(trgt.d == WEST){ // 西から来た
			IntraBox_4* find_ibox = &(my_board_1[trgt.y][trgt.x-1]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getWestNum();
			if(touch_count < 0){ *output = 11; return false; /*cout << "error! (error: 11)" << endl; exit(11);*/ }
			// コスト
			int cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			int cost_sw = (find_ibox->se) + ML + touch_count * penalty_T;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = true;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).w = true;
			}
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = true;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).w = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = true;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).w = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = true;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).w = true;
			}
		}
		if(trgt.d == NORTH){ // 北から来た
			IntraBox_4* find_ibox = &(my_board_1[trgt.y-1][trgt.x]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getNorthNum();
			if(touch_count < 0){ *output = 12; return false; /*cout << "error! (error: 12)" << endl; exit(12);*/ }
			// コスト
			int cost_ne = (find_ibox->se) + ML + touch_count * penalty_T;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			int cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = true;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).n = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = true;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).n = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = true;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).n = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = true;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).n = true;
			}
		}
		if(trgt.d == EAST){ // 東から来た
			IntraBox_4* find_ibox = &(my_board_1[trgt.y][trgt.x+1]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getEastNum();
			if(touch_count < 0){ *output = 13; return false; /*cout << "error! (error: 13)" << endl; exit(13);*/ }
			// コスト
			int cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			int cost_se = (find_ibox->sw) + ML + touch_count * penalty_T;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = true;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).e = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = true;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).e = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = true;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).e = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = true;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).e = true;
			}
		}

		if(!update) continue;
		if(trgt_box->isTypeNumber() || trgt_box->isTypeVia()) continue;

		// 北方向
		if(trgt.d!=NORTH && isInserted_1(trgt.x,trgt.y-1,start_z, board)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_1(trgt.x+1,trgt.y,start_z, board)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu[qu_tail] = next; qu_tail++;
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_1(trgt.x,trgt.y+1,start_z, board)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_1(trgt.x-1,trgt.y,start_z, board)){
			Search next = {trgt.x-1,trgt.y,EAST};
			qu[qu_tail] = next; qu_tail++;
		}
	}


	/*** シンク層の判定 ***/

	if(start_z != end_z){

		int sp_via_id = trgt_line->getSpecifiedVia();
		if(sp_via_id >= 1 && sp_via_id <= board->getViaNum()){
			Via* sp_via = board->via(sp_via_id);
			if(sp_via->getSourceZ()!=start_z || sp_via->getSinkZ()!=end_z){
				*output = 20; return false;
				/*cout << "error! (error: 20)" << endl;
				exit(20);*/
			}
			int sp_x = sp_via->getSourceX();
			int sp_y = sp_via->getSourceY();
			if(my_board_1[sp_y][sp_x].ne == INT_MAX || my_board_1[sp_y][sp_x].nw == INT_MAX ||
			my_board_1[sp_y][sp_x].se == INT_MAX || my_board_1[sp_y][sp_x].sw == INT_MAX){
				*output = 21; return false;
				/*cout << "error! (error: 21)" << endl;
				exit(21);*/
			}
		}

		for(int i=1;i<=board->getViaNum();i++){
			Via* trgt_via = board->via(i);
			if(trgt_via->getSourceZ()!=start_z || trgt_via->getSinkZ()!=end_z) continue;
			if(sp_via_id > 0 && sp_via_id != i) continue;
			start_x = trgt_via->getSourceX();
			start_y = trgt_via->getSourceY();
			start = &(my_board_2[start_y][start_x]);

			// ソース層にINT_MAXのコスト値が含まれている場合は引き継がない (issue #82)
			if(my_board_1[start_y][start_x].ne == INT_MAX || my_board_1[start_y][start_x].nw == INT_MAX ||
			my_board_1[start_y][start_x].se == INT_MAX || my_board_1[start_y][start_x].sw == INT_MAX) continue;

			start->ne = my_board_1[start_y][start_x].ne + trgt_via->getUsedLineNum() * penalty_V;
			start->nw = my_board_1[start_y][start_x].nw + trgt_via->getUsedLineNum() * penalty_V;
			start->se = my_board_1[start_y][start_x].se + trgt_via->getUsedLineNum() * penalty_V;
			start->sw = my_board_1[start_y][start_x].sw + trgt_via->getUsedLineNum() * penalty_V;

			// 北方向を探索
			if(isInserted_2(start_x,start_y-1,end_z, board)){
				Search trgt = {start_x,start_y-1,SOUTH};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 東方向を探索
			if(isInserted_2(start_x+1,start_y,end_z, board)){
				Search trgt = {start_x+1,start_y,WEST};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 南方向を探索
			if(isInserted_2(start_x,start_y+1,end_z, board)){
				Search trgt = {start_x,start_y+1,NORTH};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 西方向を探索
			if(isInserted_2(start_x-1,start_y,end_z, board)){
				Search trgt = {start_x-1,start_y,EAST};
				qu[qu_tail] = trgt; qu_tail++;
			}
		}

	}


	/*** シンク層の探索 ***/

	while (qu_head != qu_tail) {

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,end_z);
		IntraBox_4* trgt_ibox = &(my_board_2[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_4* find_ibox = &(my_board_2[trgt.y+1][trgt.x]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getSouthNum();
			if(touch_count < 0){ *output = 10; return false; /*cout << "error! (error: 10)" << endl; exit(10);*/ }
			// コスト
			int cost_se = (find_ibox->ne) + ML + touch_count * penalty_T;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			int cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = true;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).s = true;
			}
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = true;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).s = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = true;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).s = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = true;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).s = true;
			}
		}
		if(trgt.d == WEST){ // 西から来た
			IntraBox_4* find_ibox = &(my_board_2[trgt.y][trgt.x-1]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getWestNum();
			if(touch_count < 0){ *output = 11; return false; /*cout << "error! (error: 11)" << endl; exit(11);*/ }
			// コスト
			int cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			int cost_sw = (find_ibox->se) + ML + touch_count * penalty_T;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = true;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).w = true;
			}
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = true;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).w = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = true;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).w = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = true;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).w = true;
			}
		}
		if(trgt.d == NORTH){ // 北から来た
			IntraBox_4* find_ibox = &(my_board_2[trgt.y-1][trgt.x]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getNorthNum();
			if(touch_count < 0){ *output = 12; return false; /*cout << "error! (error: 12)" << endl; exit(12);*/ }
			// コスト
			int cost_ne = (find_ibox->se) + ML + touch_count * penalty_T;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			int cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = true;
				(trgt_ibox->d_ne).e = false;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).n = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = true;
				(trgt_ibox->d_se).e = false;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).n = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = true;
				(trgt_ibox->d_nw).e = false;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).n = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = true;
				(trgt_ibox->d_sw).e = false;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).n = true;
			}
		}
		if(trgt.d == EAST){ // 東から来た
			IntraBox_4* find_ibox = &(my_board_2[trgt.y][trgt.x+1]);
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getEastNum();
			if(touch_count < 0){ *output = 13; return false; /*cout << "error! (error: 13)" << endl; exit(13);*/ }
			// コスト
			int cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			int cost_se = (find_ibox->sw) + ML + touch_count * penalty_T;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
			// 北東マス
			if(cost_ne < trgt_ibox->ne){
				update = true;
				trgt_ibox->ne = cost_ne;
				(trgt_ibox->d_ne).n = false;
				(trgt_ibox->d_ne).e = true;
				(trgt_ibox->d_ne).s = false;
				(trgt_ibox->d_ne).w = false;
			}
			else if(cost_ne == trgt_ibox->ne){
				(trgt_ibox->d_ne).e = true;
			}
			// 北西マス
			if(cost_nw < trgt_ibox->nw){
				update = true;
				trgt_ibox->nw = cost_nw;
				(trgt_ibox->d_nw).n = false;
				(trgt_ibox->d_nw).e = true;
				(trgt_ibox->d_nw).s = false;
				(trgt_ibox->d_nw).w = false;
			}
			else if(cost_nw == trgt_ibox->nw){
				(trgt_ibox->d_nw).e = true;
			}
			// 南東マス
			if(cost_se < trgt_ibox->se){
				update = true;
				trgt_ibox->se = cost_se;
				(trgt_ibox->d_se).n = false;
				(trgt_ibox->d_se).e = true;
				(trgt_ibox->d_se).s = false;
				(trgt_ibox->d_se).w = false;
			}
			else if(cost_se == trgt_ibox->se){
				(trgt_ibox->d_se).e = true;
			}
			// 南西マス
			if(cost_sw < trgt_ibox->sw){
				update = true;
				trgt_ibox->sw = cost_sw;
				(trgt_ibox->d_sw).n = false;
				(trgt_ibox->d_sw).e = true;
				(trgt_ibox->d_sw).s = false;
				(trgt_ibox->d_sw).w = false;
			}
			else if(cost_sw == trgt_ibox->sw){
				(trgt_ibox->d_sw).e = true;
			}
		}

		if(!update) continue;
		if(trgt_box->isTypeNumber()) continue;

		// 北方向
		if(trgt.d!=NORTH && isInserted_2(trgt.x,trgt.y-1,end_z, board)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_2(trgt.x+1,trgt.y,end_z, board)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu[qu_tail] = next; qu_tail++;
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_2(trgt.x,trgt.y+1,end_z, board)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_2(trgt.x-1,trgt.y,end_z, board)){
			Search next = {trgt.x-1,trgt.y,EAST};
			qu[qu_tail] = next; qu_tail++;
		}
	}

#if 0
if (debug_option) { /*** デバッグ用*/
	cout << endl;
	cout << "LAYER So (line_id: " << trgt_line_id << ") (z: " << (start_z + 1) << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_4* trgt_box = &(my_board_1[y][x]);
			if(trgt_box->nw > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->nw;
			}
			if(trgt_box->ne > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->ne;
			}
			cout << " ";
		}
		cout << endl;
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_4* trgt_box = &(my_board_1[y][x]);
			if(trgt_box->sw > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->sw;
			}
			if(trgt_box->se > 10000){
				cout << " +";
			}
			else{
		 		cout << setw(2) << trgt_box->se;
		 	}
		 	cout << " ";
		}
		cout << endl;
	}
	cout << "LAYER Si (line_id: " << trgt_line_id << ") (z: " << (end_z + 1) << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_4* trgt_box = &(my_board_2[y][x]);
			if(trgt_box->nw > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->nw;
			}
			if(trgt_box->ne > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->ne;
			}
			cout << " ";
		}
		cout << endl;
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_4* trgt_box = &(my_board_2[y][x]);
			if(trgt_box->sw > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->sw;
			}
			if(trgt_box->se > 10000){
				cout << " +";
			}
			else{
		 		cout << setw(2) << trgt_box->se;
		 	}
		 	cout << " ";
		}
		cout << endl;
	}
}
#endif

	int now_x = trgt_line->getSinkX();
	int now_y = trgt_line->getSinkY();
	int intra_box = -1;
	int next_direction_array[4];
	int next_direction_array_index = 0;
	int next_count, next_id;

	/*** シンク層のバックトレース ***/

	if(start_z != end_z){

		intra_box = NE;
		for (int loop_count = 0; loop_count <= MAX_TRACKS; loop_count++) {

			Point p = {now_x, now_y, end_z};
			trgt_line->track[trgt_line->track_index] = p; (trgt_line->track_index)++;

#if 0
if( debug_option ) { cout << "(" << now_x << "," << now_y << "," << end_z << ")"; }
#endif

			if(board->box(now_x,now_y,end_z)->isTypeVia()) break;

			Direction trgt_d;
			switch(intra_box){
				case NE:
				trgt_d = my_board_2[now_y][now_x].d_ne; break;

				case NW:
				trgt_d = my_board_2[now_y][now_x].d_nw; break;

				case SE:
				trgt_d = my_board_2[now_y][now_x].d_se; break;

				case SW:
				trgt_d = my_board_2[now_y][now_x].d_sw; break;

				default:
				;//assert(!"Undefined Intra-Box"); break;
			}

			next_direction_array_index = 0;
			if(trgt_d.n) { next_direction_array[next_direction_array_index] = NORTH; next_direction_array_index++; }
			if(trgt_d.e) { next_direction_array[next_direction_array_index] = EAST; next_direction_array_index++; }
			if(trgt_d.s) { next_direction_array[next_direction_array_index] = SOUTH; next_direction_array_index++; }
			if(trgt_d.w) { next_direction_array[next_direction_array_index] = WEST; next_direction_array_index++; }
			next_count = (int)mt_genrand_int32(0, next_direction_array_index - 1);
			next_id = next_direction_array[next_count];

			switch(next_id){
				case NORTH:
				now_y = now_y - 1; // 北へ
				if(intra_box == NE || intra_box == SE) intra_box = SE;
				if(intra_box == NW || intra_box == SW) intra_box = SW;
				break;

				case EAST:
				now_x = now_x + 1; // 東へ
				if(intra_box == NE || intra_box == NW) intra_box = NW;
				if(intra_box == SE || intra_box == SW) intra_box = SW;
				break;

				case SOUTH:
				now_y = now_y + 1; // 南へ
				if(intra_box == NE || intra_box == SE) intra_box = NE;
				if(intra_box == NW || intra_box == SW) intra_box = NW;
				break;

				case WEST:
				now_x = now_x - 1; // 西へ
				if(intra_box == NE || intra_box == NW) intra_box = NE;
				if(intra_box == SE || intra_box == SW) intra_box = SE;
				break;
			}
		}
	}


	/*** ソース層のバックトレース ***/

	intra_box = NE;
	for (int loop_count = 0; loop_count <= MAX_TRACKS; loop_count++) {

		Point p = {now_x, now_y, start_z};
		trgt_line->track[trgt_line->track_index] = p; (trgt_line->track_index)++;

#if 0
if( debug_option ){ cout << "(" << now_x << "," << now_y << "," << start_z << ")"; }
#endif

		if(now_x==trgt_line->getSourceX() && now_y==trgt_line->getSourceY()) break;

		Direction trgt_d;
		switch(intra_box){
			case NE:
			trgt_d = my_board_1[now_y][now_x].d_ne; break;

			case NW:
			trgt_d = my_board_1[now_y][now_x].d_nw; break;

			case SE:
			trgt_d = my_board_1[now_y][now_x].d_se; break;

			case SW:
			trgt_d = my_board_1[now_y][now_x].d_sw; break;

			default:
			;//assert(!"Undefined Intra-Box"); break;
		}

		next_direction_array_index = 0;
		if(trgt_d.n) { next_direction_array[next_direction_array_index] = NORTH; next_direction_array_index++; }
		if(trgt_d.e) { next_direction_array[next_direction_array_index] = EAST; next_direction_array_index++; }
		if(trgt_d.s) { next_direction_array[next_direction_array_index] = SOUTH; next_direction_array_index++; }
		if(trgt_d.w) { next_direction_array[next_direction_array_index] = WEST; next_direction_array_index++; }
		next_count = (int)mt_genrand_int32(0, next_direction_array_index - 1);
		next_id = next_direction_array[next_count];

		switch(next_id){
			case NORTH:
			now_y = now_y - 1; // 北へ
			if(intra_box == NE || intra_box == SE) intra_box = SE;
			if(intra_box == NW || intra_box == SW) intra_box = SW;
			break;

			case EAST:
			now_x = now_x + 1; // 東へ
			if(intra_box == NE || intra_box == NW) intra_box = NW;
			if(intra_box == SE || intra_box == SW) intra_box = SW;
			break;

			case SOUTH:
			now_y = now_y + 1; // 南へ
			if(intra_box == NE || intra_box == SE) intra_box = NE;
			if(intra_box == NW || intra_box == SW) intra_box = NW;
			break;

			case WEST:
			now_x = now_x - 1; // 西へ
			if(intra_box == NE || intra_box == NW) intra_box = NE;
			if(intra_box == SE || intra_box == SW) intra_box = SE;
			break;
		}
	}

#if 0
if( debug_option ) { cout << endl; }
#endif


	/*** ターゲットラインのトラックを整理 ***/

	bool retry = true;
	while(retry){
		retry = false;

		// トラックを一時退避
		Point tmp_track[MAX_TRACKS];
		int tmp_track_index = 0;
		for (int i = 0; i < trgt_line->track_index; i++) {
			tmp_track[tmp_track_index] = trgt_line->track[i];
			tmp_track_index++;
		}

		// 冗長部分を排除してトラックを整理
		trgt_line->track_index = 0;
		for (int i = 0; i < tmp_track_index; i++) {
			if (tmp_track_index - 2 <= i) {
				trgt_line->track[trgt_line->track_index] = tmp_track[i];
				(trgt_line->track_index)++;
				continue;
			}
			if (tmp_track[i].x == tmp_track[i + 2].x && tmp_track[i].y == tmp_track[i + 2].y && tmp_track[i].z == tmp_track[i+2].z) {
				retry = true;
				i++;
				continue;
			}
			trgt_line->track[trgt_line->track_index] = tmp_track[i];
			(trgt_line->track_index)++;
		}
	}

	// delete は高位合成できないから放置。
	/*for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			delete my_board_1[y][x];
			delete my_board_2[y][x];
		}
	}*/

	*output = 0;
	return true;
}

bool isInserted_1(int x,int y,int z, Board *board){ // ソース層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia()) return false;

	return true;
}

bool isInserted_2(int x,int y,int z, Board *board){ // シンク層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia() || trgt_box->isTypeVia()) return false;

	return true;
}

int countLine(int x,int y,int z, Board *board){

	Box* trgt_box = board->box(x,y,z);

	int count = 0;
	count += trgt_box->getNorthNum();
	count += trgt_box->getEastNum();
	count += trgt_box->getSouthNum();
	count += trgt_box->getWestNum();
	//assert(0 <= count);

	return count/2;
}

#if 0
void recordLine(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();

	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int old_z = trgt_line->getSinkZ();
	int new_x = (*trgt_track)[1].x;
	int new_y = (*trgt_track)[1].y;
	int new_z = (*trgt_track)[1].z;
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

	for(int i=2;i<(int)(trgt_track->size()-1);i++){
		new_x = (*trgt_track)[i].x;
		new_y = (*trgt_track)[i].y;
		new_z = (*trgt_track)[i].z;
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

void deleteLine(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();

	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int old_z = trgt_line->getSinkZ();
	int new_x = (*trgt_track)[1].x;
	int new_y = (*trgt_track)[1].y;
	int new_z = (*trgt_track)[1].z;
	Box* old_box = board->box(old_x,old_y,old_z);
	Box* new_box = board->box(new_x,new_y,new_z);

	if(new_box->isTypeVia());
	else if(new_x==old_x && new_y==old_y-1){ // 北
		new_box->decrementSouthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		new_box->decrementWestNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		new_box->decrementNorthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		new_box->decrementEastNum();
	}

	old_x = new_x; old_y = new_y; old_z = new_z;
	old_box = new_box;

	for(int i=2;i<(int)(trgt_track->size()-1);i++){
		new_x = (*trgt_track)[i].x;
		new_y = (*trgt_track)[i].y;
		new_z = (*trgt_track)[i].z;
		new_box = board->box(new_x,new_y,new_z);

		if(new_box->isTypeVia() && old_box->isTypeVia()){
			board->via(old_box->getIndex())->decrementUsedLineNum();
		}
		else{
			if(new_x==old_x && new_y==old_y-1){ // 北
				if(old_box->isTypeBlank()) old_box->decrementNorthNum();
				if(new_box->isTypeBlank()) new_box->decrementSouthNum();
			}
			else if(new_x==old_x+1 && new_y==old_y){ // 東
				if(old_box->isTypeBlank()) old_box->decrementEastNum();
				if(new_box->isTypeBlank()) new_box->decrementWestNum();
			}
			else if(new_x==old_x && new_y==old_y+1){ // 南
				if(old_box->isTypeBlank()) old_box->decrementSouthNum();
				if(new_box->isTypeBlank()) new_box->decrementNorthNum();
			}
			else if(new_x==old_x-1 && new_y==old_y){ // 西
				if(old_box->isTypeBlank()) old_box->decrementWestNum();
				if(new_box->isTypeBlank()) new_box->decrementEastNum();
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
		old_box->decrementNorthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		old_box->decrementEastNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		old_box->decrementSouthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		old_box->decrementWestNum();
	}
}
#endif

#if 0
bool final_routing(int trgt_line_id, bool debug_option){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->clearTrack();

	// ボードの初期化
	vector<vector<IntraBox_1*> > my_board_1(board->getSizeY(), vector<IntraBox_1*>(board->getSizeX())); // ソース側のボード
	vector<vector<IntraBox_1*> > my_board_2(board->getSizeY(), vector<IntraBox_1*>(board->getSizeX())); // シンク側のボード
	IntraBox_1 init = { INT_MAX, {false,false,false,false,NOT_USE,NOT_USE,NOT_USE,NOT_USE} };
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			my_board_1[y][x] = new IntraBox_1;
			*(my_board_1[y][x]) = init;
			my_board_2[y][x] = new IntraBox_1;
			*(my_board_2[y][x]) = init;
		}
	}
	int start_x, start_y;
	IntraBox_1* start;
	queue<Search> qu;

	int start_z = trgt_line->getSourceZ();
	int end_z = trgt_line->getSinkZ();


	/*** ソース層の探索 ***/

	// スタート地点の設定
	start_x = trgt_line->getSourceX();
	start_y = trgt_line->getSourceY();
	start = my_board_1[start_y][start_x];
	start->cost = 0;

	// 北方向を探索
	if(isInserted_3(start_x,start_y-1,start_z)){
		Search trgt = {start_x,start_y-1,SOUTH};
		qu.push(trgt);
	}
	// 東方向を探索
	if(isInserted_3(start_x+1,start_y,start_z)){
		Search trgt = {start_x+1,start_y,WEST};
		qu.push(trgt);
	}
	// 南方向を探索
	if(isInserted_3(start_x,start_y+1,start_z)){
		Search trgt = {start_x,start_y+1,NORTH};
		qu.push(trgt);
	}
	// 西方向を探索
	if(isInserted_3(start_x-1,start_y,start_z)){
		Search trgt = {start_x-1,start_y,EAST};
		qu.push(trgt);
	}

	while(!qu.empty()){

		Search trgt = qu.front();
		qu.pop();

		Box* trgt_box = board->box(trgt.x,trgt.y,start_z);
		IntraBox_1* trgt_ibox = my_board_1[trgt.y][trgt.x];

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_1* find_ibox = my_board_1[trgt.y+1][trgt.x];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).s){ previous = SOUTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_s = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).c_s = previous;
			}
		}
		if(trgt.d == WEST){ // 西から来た
			IntraBox_1* find_ibox = my_board_1[trgt.y][trgt.x-1];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).w){ previous = WEST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).c_w = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).c_w = previous;
			}
		}
		if(trgt.d == NORTH){ // 北から来た
			IntraBox_1* find_ibox = my_board_1[trgt.y-1][trgt.x];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).n){ previous = NORTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_n = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).c_n = previous;
			}
		}
		if(trgt.d == EAST){ // 東から来た
			IntraBox_1* find_ibox = my_board_1[trgt.y][trgt.x+1];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).e){ previous = EAST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_e = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).c_e = previous;
			}
		}

		if(!update) continue;
		if(trgt_box->isTypeNumber() || trgt_box->isTypeVia()) continue;

		// 北方向
		if(trgt.d!=NORTH && isInserted_3(trgt.x,trgt.y-1,start_z)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_3(trgt.x+1,trgt.y,start_z)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_3(trgt.x,trgt.y+1,start_z)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_3(trgt.x-1,trgt.y,start_z)){
			Search next = {trgt.x-1,trgt.y,EAST};
			qu.push(next);
		}
	}


	/*** シンク層の判定 ***/

	if(start_z != end_z){
		for(int i=1;i<=board->getViaNum();i++){
			Via* trgt_via = board->via(i);
			if(trgt_via->getSourceZ()!=start_z || trgt_via->getSinkZ()!=end_z) continue;
			start_x = trgt_via->getSourceX();
			start_y = trgt_via->getSourceY();
			start = my_board_2[start_y][start_x];

			if(my_board_1[start_y][start_x]->cost == INT_MAX) continue;
			if(trgt_via->getUsedLineNum() > 0) continue;

			start->cost = my_board_1[start_y][start_x]->cost;

			// 北方向を探索
			if(isInserted_4(start_x,start_y-1,end_z)){
				Search trgt = {start_x,start_y-1,SOUTH};
				qu.push(trgt);
			}
			// 東方向を探索
			if(isInserted_4(start_x+1,start_y,end_z)){
				Search trgt = {start_x+1,start_y,WEST};
				qu.push(trgt);
			}
			// 南方向を探索
			if(isInserted_4(start_x,start_y+1,end_z)){
				Search trgt = {start_x,start_y+1,NORTH};
				qu.push(trgt);
			}
			// 西方向を探索
			if(isInserted_4(start_x-1,start_y,end_z)){
				Search trgt = {start_x-1,start_y,EAST};
				qu.push(trgt);
			}
		}

	}


	/*** シンク層の探索 ***/

	while(!qu.empty()){

		Search trgt = qu.front();
		qu.pop();

		Box* trgt_box = board->box(trgt.x,trgt.y,end_z);
		IntraBox_1* trgt_ibox = my_board_2[trgt.y][trgt.x];

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_1* find_ibox = my_board_2[trgt.y+1][trgt.x];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).s){ previous = SOUTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_s = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).s = true;
				(trgt_ibox->d).c_s = previous;
			}
		}
		if(trgt.d == WEST){ // 西から来た
			IntraBox_1* find_ibox = my_board_2[trgt.y][trgt.x-1];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).w){ previous = WEST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).c_w = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).w = true;
				(trgt_ibox->d).c_w = previous;
			}
		}
		if(trgt.d == NORTH){ // 北から来た
			IntraBox_1* find_ibox = my_board_2[trgt.y-1][trgt.x];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).n){ previous = NORTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).e = false;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_n = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).n = true;
				(trgt_ibox->d).c_n = previous;
			}
		}
		if(trgt.d == EAST){ // 東から来た
			IntraBox_1* find_ibox = my_board_2[trgt.y][trgt.x+1];
			// 折れ曲がりあり？
			int turn_count = 0;
			int previous = NOT_USE;
			if((find_ibox->d).e){ previous = EAST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			int cost = (find_ibox->cost) + ML + turn_count * BT;
			if(cost < trgt_ibox->cost){
				update = true;
				trgt_ibox->cost = cost;
				(trgt_ibox->d).n = false;
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).s = false;
				(trgt_ibox->d).w = false;
				(trgt_ibox->d).c_e = previous;
			}
			else if(cost == trgt_ibox->cost){
				(trgt_ibox->d).e = true;
				(trgt_ibox->d).c_e = previous;
			}
		}

		if(!update) continue;
		if(trgt_box->isTypeNumber()) continue;

		// 北方向
		if(trgt.d!=NORTH && isInserted_4(trgt.x,trgt.y-1,end_z)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_4(trgt.x+1,trgt.y,end_z)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_4(trgt.x,trgt.y+1,end_z)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_4(trgt.x-1,trgt.y,end_z)){
			Search next = {trgt.x-1,trgt.y,EAST};
			qu.push(next);
		}
	}

if (debug_option) { /*** デバッグ用*/
	cout << endl;
	cout << "LAYER So (line_id: " << trgt_line_id << ") (z: " << (start_z + 1) << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_1* trgt_box = my_board_1[y][x];
			if(trgt_box->cost > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->cost;
			}
			cout << " ";
		}
		cout << endl;
	}
	cout << "LAYER Si (line_id: " << trgt_line_id << ") (z: " << (end_z + 1) << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox_1* trgt_box = my_board_2[y][x];
			if(trgt_box->cost > 10000){
				cout << " +";
			}
			else{
				cout << setw(2) << trgt_box->cost;
			}
			cout << " ";
		}
		cout << endl;
	}
}

	int now_x = trgt_line->getSinkX();
	int now_y = trgt_line->getSinkY();
	vector<int> next_direction_array;
	int next_count, next_id;

	Direction_R trgt_d;

	/*** シンク層のバックトレース ***/

	if(start_z != end_z){

		Point p_s = {now_x, now_y, end_z};
		trgt_line->pushPointToTrack(p_s);

if (debug_option) { cout << "(" << now_x << "," << now_y << "," << end_z << ")"; }

		trgt_d = my_board_2[now_y][now_x]->d;
		next_direction_array.clear();
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
		next_id = next_direction_array[next_count];
		switch(next_id){
			case NORTH: // 北へ
			now_y = now_y - 1; next_id = trgt_d.c_n; break;
			case EAST:  // 東へ
			now_x = now_x + 1; next_id = trgt_d.c_e; break;
			case SOUTH: // 南へ
			now_y = now_y + 1; next_id = trgt_d.c_s; break;
			case WEST:  // 西へ
			now_x = now_x - 1; next_id = trgt_d.c_w; break;
		}

		while(1){

			Point p = {now_x, now_y, end_z};
			trgt_line->pushPointToTrack(p);

if (debug_option) { cout << "(" << now_x << "," << now_y << "," << end_z << ")"; }

			if(board->box(now_x,now_y,end_z)->isTypeVia()) break;

			trgt_d = my_board_2[now_y][now_x]->d;

			switch(next_id){
				case NORTH:
				if(!trgt_d.n) next_id = -1;
				break;
				case EAST:
				if(!trgt_d.e) next_id = -1;
				break;
				case SOUTH:
				if(!trgt_d.s) next_id = -1;
				break;
				case WEST:
				if(!trgt_d.w) next_id = -1;
				break;
			}

			if(next_id < 0){ *output = 51; return false; /*cout << "error! (error: 51)" << endl; exit(51);*/ }

			switch(next_id){
				case NORTH: // 北へ
				now_y = now_y - 1; next_id = trgt_d.c_n; break;
				case EAST:  // 東へ
				now_x = now_x + 1; next_id = trgt_d.c_e; break;
				case SOUTH: // 南へ
				now_y = now_y + 1; next_id = trgt_d.c_s; break;
				case WEST:  // 西へ
				now_x = now_x - 1; next_id = trgt_d.c_w; break;
			}
		}
	}


	/*** ソース層のバックトレース ***/

	Point p_v = {now_x, now_y, start_z};
	trgt_line->pushPointToTrack(p_v);

if (debug_option) { cout << "(" << now_x << "," << now_y << "," << start_z << ")"; }

	trgt_d = my_board_1[now_y][now_x]->d;
	next_direction_array.clear();
	if(trgt_d.n) next_direction_array.push_back(NORTH);
	if(trgt_d.e) next_direction_array.push_back(EAST);
	if(trgt_d.s) next_direction_array.push_back(SOUTH);
	if(trgt_d.w) next_direction_array.push_back(WEST);
	next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
	next_id = next_direction_array[next_count];
	switch(next_id){
		case NORTH: // 北へ
		now_y = now_y - 1; next_id = trgt_d.c_n; break;
		case EAST:  // 東へ
		now_x = now_x + 1; next_id = trgt_d.c_e; break;
		case SOUTH: // 南へ
		now_y = now_y + 1; next_id = trgt_d.c_s; break;
		case WEST:  // 西へ
		now_x = now_x - 1; next_id = trgt_d.c_w; break;
	}

	while(1){

		Point p = {now_x, now_y, start_z};
		trgt_line->pushPointToTrack(p);

if (debug_option) { cout << "(" << now_x << "," << now_y << "," << start_z << ")"; }

		if(now_x==trgt_line->getSourceX() && now_y==trgt_line->getSourceY()) break;

		trgt_d = my_board_1[now_y][now_x]->d;

		switch(next_id){
			case NORTH:
			if(!trgt_d.n) next_id = -1;
			break;
			case EAST:
			if(!trgt_d.e) next_id = -1;
			break;
			case SOUTH:
			if(!trgt_d.s) next_id = -1;
			break;
			case WEST:
			if(!trgt_d.w) next_id = -1;
			break;
		}

		if(next_id < 0){ *output = 52; return false; /*cout << "error! (error: 52)" << endl; exit(52);*/ }

		switch(next_id){
			case NORTH: // 北へ
			now_y = now_y - 1; next_id = trgt_d.c_n; break;
			case EAST:  // 東へ
			now_x = now_x + 1; next_id = trgt_d.c_e; break;
			case SOUTH: // 南へ
			now_y = now_y + 1; next_id = trgt_d.c_s; break;
			case WEST:  // 西へ
			now_x = now_x - 1; next_id = trgt_d.c_w; break;
		}
	}

if( debug_option ) { cout << endl; }

	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			delete my_board_1[y][x];
			delete my_board_2[y][x];
		}
	}

	*output = 0;
	return true;
}

bool isInserted_3(int x,int y,int z){ // ソース層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia()) return false;
	if(countLine(x,y,z) > 0) return false; // ラインがあるマスは探索しない

	return true;
}

bool isInserted_4(int x,int y,int z){ // シンク層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia() || trgt_box->isTypeVia()) return false;
	if(countLine(x,y,z) > 0) return false; // ラインがあるマスは探索しない

	return true;
}
#endif
