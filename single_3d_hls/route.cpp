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


bool routing(const ap_int<8> trgt_line_id, const ap_uint<16> penalty_T, const ap_uint<16> penalty_C, const ap_uint<16> penalty_V, Board *board, ap_int<8> *output){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->track_index = 0;

	// ボードの初期化
	IntraBox_4 my_board_1[MAX_BOXES][MAX_BOXES]; // ソース側のボード
//#pragma HLS ARRAY_PARTITION variable=my_board_1 complete dim=1
	IntraBox_4 my_board_2[MAX_BOXES][MAX_BOXES]; // シンク側のボード
//#pragma HLS ARRAY_PARTITION variable=my_board_2 complete dim=2
	IntraBox_4 init = {
		COST_MAX,COST_MAX,COST_MAX,COST_MAX,
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false}};
	for (ap_int<7> y = 0; y < board->getSizeY(); y++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
		for (ap_int<7> x = 0; x < board->getSizeX(); x++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
////#pragma HLS PIPELINE
			my_board_1[y][x] = init;
			my_board_2[y][x] = init;
		}
	}
	ap_int<7> start_x, start_y;
	IntraBox_4* start;
	Search qu[MAX_SEARCH];
//#pragma HLS ARRAY_PARTITION variable=qu cyclic factor=20 dim=0
	ap_int<16> qu_head, qu_tail;

	ap_int<5> start_z = trgt_line->getSourceZ();
	ap_int<5> end_z = trgt_line->getSinkZ();


/* ********************************
 * Phase 1: ソース層の探索
 ******************************** */

	qu_head = 0;
	qu_tail = 0;

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
#pragma HLS LOOP_TRIPCOUNT min=100 max=100 avg=100
//#pragma HLS PIPELINE // これONにするとメモリ使用率100%になる

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,start_z);
		IntraBox_4* trgt_ibox = &(my_board_1[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_4* find_ibox = &(my_board_1[trgt.y+1][trgt.x]);
			// タッチ数
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getSouthNum();
			if(touch_count < 0){ *output = 10; return false; /*cout << "error! (error: 10)" << endl; exit(10);*/ }
			// コスト
			ap_int<16> cost_se = (find_ibox->ne) + ML + touch_count * penalty_T;
			ap_int<16> cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			ap_int<16> cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T;
			ap_int<16> cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getWestNum();
			if(touch_count < 0){ *output = 11; return false; /*cout << "error! (error: 11)" << endl; exit(11);*/ }
			// コスト
			ap_int<16> cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T;
			ap_int<16> cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			ap_int<16> cost_sw = (find_ibox->se) + ML + touch_count * penalty_T;
			ap_int<16> cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getNorthNum();
			if(touch_count < 0){ *output = 12; return false; /*cout << "error! (error: 12)" << endl; exit(12);*/ }
			// コスト
			ap_int<16> cost_ne = (find_ibox->se) + ML + touch_count * penalty_T;
			ap_int<16> cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			ap_int<16> cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T;
			ap_int<16> cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,start_z, board) - trgt_box->getEastNum();
			if(touch_count < 0){ *output = 13; return false; /*cout << "error! (error: 13)" << endl; exit(13);*/ }
			// コスト
			ap_int<16> cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T;
			ap_int<16> cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			ap_int<16> cost_se = (find_ibox->sw) + ML + touch_count * penalty_T;
			ap_int<16> cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
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


/* ********************************
 * Phase 2-1: シンク層の判定
 ******************************** */

	if(start_z != end_z){

		qu_head = 0;
		qu_tail = 0;

		ap_int<8> sp_via_id = trgt_line->getSpecifiedVia();
		if(sp_via_id >= 1 && sp_via_id <= board->getViaNum()){
			Via* sp_via = board->via(sp_via_id);
			if(sp_via->getSourceZ()!=start_z || sp_via->getSinkZ()!=end_z){
				*output = 20; return false;
				/*cout << "error! (error: 20)" << endl;
				exit(20);*/
			}
			ap_int<7> sp_x = sp_via->getSourceX();
			ap_int<7> sp_y = sp_via->getSourceY();
			if(my_board_1[sp_y][sp_x].ne == COST_MAX || my_board_1[sp_y][sp_x].nw == COST_MAX ||
			my_board_1[sp_y][sp_x].se == COST_MAX || my_board_1[sp_y][sp_x].sw == COST_MAX){
				*output = 21; return false;
				/*cout << "error! (error: 21)" << endl;
				exit(21);*/
			}
		}

		for(ap_int<8> i=1;i<=board->getViaNum();i++){
#pragma HLS LOOP_TRIPCOUNT min=5 max=45 avg=25
////#pragma HLS PIPELINE
			Via* trgt_via = board->via(i);
			if(trgt_via->getSourceZ()!=start_z || trgt_via->getSinkZ()!=end_z) continue;
			if(sp_via_id > 0 && sp_via_id != i) continue;
			start_x = trgt_via->getSourceX();
			start_y = trgt_via->getSourceY();
			start = &(my_board_2[start_y][start_x]);

			// ソース層に COST_MAX のコスト値が含まれている場合は引き継がない (issue #82)
			if(my_board_1[start_y][start_x].ne == COST_MAX || my_board_1[start_y][start_x].nw == COST_MAX ||
			my_board_1[start_y][start_x].se == COST_MAX || my_board_1[start_y][start_x].sw == COST_MAX) continue;

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


/* ********************************
 * Phase 2-2: シンク層の探索
 ******************************** */

	while (qu_head != qu_tail) {
#pragma HLS LOOP_TRIPCOUNT min=100 max=100 avg=100
//#pragma HLS PIPELINE // これONにするとメモリ使用率100%になる

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,end_z);
		IntraBox_4* trgt_ibox = &(my_board_2[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_4* find_ibox = &(my_board_2[trgt.y+1][trgt.x]);
			// タッチ数
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getSouthNum();
			if(touch_count < 0){ *output = 10; return false; /*cout << "error! (error: 10)" << endl; exit(10);*/ }
			// コスト
			ap_int<16> cost_se = (find_ibox->ne) + ML + touch_count * penalty_T;
			ap_int<16> cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			ap_int<16> cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T;
			ap_int<16> cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getWestNum();
			if(touch_count < 0){ *output = 11; return false; /*cout << "error! (error: 11)" << endl; exit(11);*/ }
			// コスト
			ap_int<16> cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T;
			ap_int<16> cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			ap_int<16> cost_sw = (find_ibox->se) + ML + touch_count * penalty_T;
			ap_int<16> cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getNorthNum();
			if(touch_count < 0){ *output = 12; return false; /*cout << "error! (error: 12)" << endl; exit(12);*/ }
			// コスト
			ap_int<16> cost_ne = (find_ibox->se) + ML + touch_count * penalty_T;
			ap_int<16> cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C;
			ap_int<16> cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T;
			ap_int<16> cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C;
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
			ap_int<8> touch_count = countLine(trgt.x,trgt.y,end_z, board) - trgt_box->getEastNum();
			if(touch_count < 0){ *output = 13; return false; /*cout << "error! (error: 13)" << endl; exit(13);*/ }
			// コスト
			ap_int<16> cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T;
			ap_int<16> cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C;
			ap_int<16> cost_se = (find_ibox->sw) + ML + touch_count * penalty_T;
			ap_int<16> cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C;
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

	ap_int<7> now_x = trgt_line->getSinkX();
	ap_int<7> now_y = trgt_line->getSinkY();
	ap_uint<2> intra_box;
	ap_uint<2> next_direction_array[4];
#pragma HLS ARRAY_PARTITION variable=next_direction_array complete dim=0
	ap_uint<3> next_direction_array_index = 0;
	ap_int<8> next_count; ap_uint<2> next_id;


/* ********************************
 * Phase 3-1: シンク層のバックトレース
 ******************************** */

	if(start_z != end_z){

		intra_box = NE;
		for (ap_uint<8> loop_count = 0; loop_count <= MAX_TRACKS; loop_count++) {
//#pragma HLS PIPELINE // ONにするとLUTリソースオーバーする
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
			next_count = /*(int)*/mt_genrand_int32(0, next_direction_array_index - 1);
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


/* ********************************
 * Phase 3-2: ソース層のバックトレース
 ******************************** */

	intra_box = NE;
	for (ap_uint<8> loop_count = 0; loop_count <= MAX_TRACKS; loop_count++) {
//#pragma HLS PIPELINE // ONにするとLUTリソースオーバーする

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
		next_count = /*(int)*/mt_genrand_int32(0, next_direction_array_index - 1);
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


/* ********************************
 * Phase 4: ターゲットラインのトラックを整理
 ******************************** */

	routing_arrange(trgt_line);

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

void routing_arrange(Line *trgt_line) {
//#pragma HLS INLINE // ONにするとレイテンシ増える

	bool retry = true;
	while(retry){
#pragma HLS LOOP_TRIPCOUNT min=10 max=10 avg=10

		retry = false;

		// トラックを一時退避
		Point tmp_track[MAX_TRACKS];
////#pragma HLS ARRAY_PARTITION variable=tmp_track complete dim=0
		int tmp_track_index = 0;
		for (ap_uint<8> i = 0; i < trgt_line->track_index; i++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=160 avg=40
////#pragma HLS PIPELINE
			tmp_track[tmp_track_index] = trgt_line->track[i];
			tmp_track_index++;
		}

		// 冗長部分を排除してトラックを整理
		trgt_line->track_index = 0;
		for (ap_uint<8> i = 0; i < tmp_track_index; i++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=160 avg=40
////#pragma HLS PIPELINE
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
}

bool isInserted_1(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board){ // ソース層用
//#pragma HLS INLINE // ONにするとクリティカルパスが増える

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia()) return false;

	return true;
}

bool isInserted_2(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board){ // シンク層用
//#pragma HLS INLINE // ONにするとクリティカルパスが増える

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia() || trgt_box->isTypeVia()) return false;

	return true;
}

int countLine(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board){
//#pragma HLS INLINE // ONにするとクリティカルパスが増える

	Box* trgt_box = board->box(x,y,z);

	ap_int<8> count = 0;
	count += trgt_box->getNorthNum();
	count += trgt_box->getEastNum();
	count += trgt_box->getSouthNum();
	count += trgt_box->getWestNum();
	//assert(0 <= count);

	return count/2;
}

void recordLine(ap_int<8> trgt_line_id, Board *board){

	Line* trgt_line = board->line(trgt_line_id);
	Point *trgt_track = trgt_line->track;

	ap_int<7> old_x = trgt_line->getSinkX();
	ap_int<7> old_y = trgt_line->getSinkY();
	ap_int<5> old_z = trgt_line->getSinkZ();
	ap_int<7> new_x = (trgt_track[1]).x; //int new_x = (trgt_track + 1)->x;
	ap_int<7> new_y = (trgt_track[1]).y; //int new_y = (trgt_track + 1)->y;
	ap_int<5> new_z = (trgt_track[1]).z; //int new_z = (trgt_track + 1)->z;
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

	for (ap_uint<8> i = 2; i < (trgt_line->track_index) - 1; i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=160 avg=40
//#pragma HLS PIPELINE

		new_x = (trgt_track[i]).x; //new_x = (trgt_track + i)->x;
		new_y = (trgt_track[i]).y; //new_y = (trgt_track + i)->y;
		new_z = (trgt_track[i]).z; //new_z = (trgt_track + i)->z;
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

void deleteLine(ap_int<8> trgt_line_id, Board *board) {

	Line* trgt_line = board->line(trgt_line_id);
	Point *trgt_track = trgt_line->track;

	ap_int<7> old_x = trgt_line->getSinkX();
	ap_int<7> old_y = trgt_line->getSinkY();
	ap_int<5> old_z = trgt_line->getSinkZ();
	ap_int<7> new_x = (trgt_track[1]).x;
	ap_int<7> new_y = (trgt_track[1]).y;
	ap_int<5> new_z = (trgt_track[1]).z;
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

	for (ap_uint<8> i = 2; i < trgt_line->track_index - 1; i++) {
#pragma HLS LOOP_TRIPCOUNT min=2 max=160 avg=40
//#pragma HLS PIPELINE

		new_x = (trgt_track)[i].x;
		new_y = (trgt_track)[i].y;
		new_z = (trgt_track)[i].z;
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
