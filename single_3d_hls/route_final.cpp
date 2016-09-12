/**
 * route_final.cpp
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
#include "./route_final.hpp"


bool final_routing(const ap_int<8> trgt_line_id, Board *board, ap_int<8> *output){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->track_index = 0;

	// ボードの初期化
	IntraBox_1 my_board_1[MAX_BOXES][MAX_BOXES]; // ソース側のボード
//#pragma HLS ARRAY_PARTITION cyclic factor=10 variable=my_board_1 dim=2
	IntraBox_1 my_board_2[MAX_BOXES][MAX_BOXES]; // シンク側のボード
//#pragma HLS ARRAY_PARTITION cyclic factor=10 variable=my_board_2 dim=2
	IntraBox_1 init = {
		COST_MAX,
		{false,false,false,false,NOT_USE,NOT_USE,NOT_USE,NOT_USE} };
	for (ap_int<7> y = 0; y < board->getSizeY(); y++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
		for(ap_int<7> x = 0; x < board->getSizeX(); x++) {
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
////#pragma HLS PIPELINE
			my_board_1[y][x] = init;
			my_board_2[y][x] = init;
		}
	}
	ap_int<7> start_x, start_y;
	IntraBox_1* start;
	Search qu[MAX_SEARCH];
//#pragma HLS ARRAY_PARTITION variable=qu cyclic factor=10 dim=0
	ap_int<32> qu_head = 0;
	ap_int<32> qu_tail = 0;

	ap_int<5> start_z = trgt_line->getSourceZ();
	ap_int<5> end_z = trgt_line->getSinkZ();


/* ********************************
 * Phase 1: ソース層の探索
 ******************************** */

	// スタート地点の設定
	start_x = trgt_line->getSourceX();
	start_y = trgt_line->getSourceY();
	start = &(my_board_1[start_y][start_x]);
	start->cost = 0;

	// 北方向を探索
	if(isInserted_3(start_x,start_y-1,start_z, board)){
		Search trgt = {start_x,start_y-1,SOUTH};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 東方向を探索
	if(isInserted_3(start_x+1,start_y,start_z, board)){
		Search trgt = {start_x+1,start_y,WEST};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 南方向を探索
	if(isInserted_3(start_x,start_y+1,start_z, board)){
		Search trgt = {start_x,start_y+1,NORTH};
		qu[qu_tail] = trgt; qu_tail++;
	}
	// 西方向を探索
	if(isInserted_3(start_x-1,start_y,start_z, board)){
		Search trgt = {start_x-1,start_y,EAST};
		qu[qu_tail] = trgt; qu_tail++;
	}

	while (qu_head != qu_tail) {
#pragma HLS LOOP_TRIPCOUNT min=100 max=100 avg=100
//#pragma HLS PIPELINE // ONにするといろいろだめ

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,start_z);
		IntraBox_1* trgt_ibox = &(my_board_1[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_1* find_ibox = &(my_board_1[trgt.y+1][trgt.x]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).s){ previous = SOUTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_1[trgt.y][trgt.x-1]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).w){ previous = WEST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_1[trgt.y-1][trgt.x]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).n){ previous = NORTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_1[trgt.y][trgt.x+1]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).e){ previous = EAST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
		if(trgt.d!=NORTH && isInserted_3(trgt.x,trgt.y-1,start_z, board)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_3(trgt.x+1,trgt.y,start_z, board)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu[qu_tail] = next; qu_tail++;
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_3(trgt.x,trgt.y+1,start_z, board)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_3(trgt.x-1,trgt.y,start_z, board)){
			Search next = {trgt.x-1,trgt.y,EAST};
			qu[qu_tail] = next; qu_tail++;
		}
	}


/* ********************************
 * Phase 2-1: シンク層の判定
 ******************************** */

	if(start_z != end_z){

		for (ap_int<8> i = 1; i <= board->getViaNum(); i++) {
#pragma HLS LOOP_TRIPCOUNT min=5 max=45 avg=25
////#pragma HLS PIPELINE
			Via* trgt_via = board->via(i);
			if(trgt_via->getSourceZ()!=start_z || trgt_via->getSinkZ()!=end_z) continue;
			start_x = trgt_via->getSourceX();
			start_y = trgt_via->getSourceY();
			start = &(my_board_2[start_y][start_x]);

			if(my_board_1[start_y][start_x].cost == COST_MAX) continue;
			if(trgt_via->getUsedLineNum() > 0) continue;

			start->cost = my_board_1[start_y][start_x].cost;

			// 北方向を探索
			if(isInserted_4(start_x,start_y-1,end_z, board)){
				Search trgt = {start_x,start_y-1,SOUTH};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 東方向を探索
			if(isInserted_4(start_x+1,start_y,end_z, board)){
				Search trgt = {start_x+1,start_y,WEST};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 南方向を探索
			if(isInserted_4(start_x,start_y+1,end_z, board)){
				Search trgt = {start_x,start_y+1,NORTH};
				qu[qu_tail] = trgt; qu_tail++;
			}
			// 西方向を探索
			if(isInserted_4(start_x-1,start_y,end_z, board)){
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
//#pragma HLS PIPELINE // ONにするといろいろだめ

		Search trgt = qu[qu_head];
		qu_head++;

		Box* trgt_box = board->box(trgt.x,trgt.y,end_z);
		IntraBox_1* trgt_ibox = &(my_board_2[trgt.y][trgt.x]);

		bool update = false; // コストの更新があったか？
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox_1* find_ibox = &(my_board_2[trgt.y+1][trgt.x]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).s){ previous = SOUTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_2[trgt.y][trgt.x-1]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).w){ previous = WEST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_2[trgt.y-1][trgt.x]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).n){ previous = NORTH; }
			else if((find_ibox->d).e){ previous = EAST; turn_count++; }
			else if((find_ibox->d).w){ previous = WEST; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
			IntraBox_1* find_ibox = &(my_board_2[trgt.y][trgt.x+1]);
			// 折れ曲がりあり？
			ap_int<8> turn_count = 0;
			ap_int<8> previous = NOT_USE;
			if((find_ibox->d).e){ previous = EAST; }
			else if((find_ibox->d).n){ previous = NORTH; turn_count++; }
			else if((find_ibox->d).s){ previous = SOUTH; turn_count++; }
			// コスト
			ap_int<16> cost = (find_ibox->cost) + ML + turn_count * BT;
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
		if(trgt.d!=NORTH && isInserted_4(trgt.x,trgt.y-1,end_z, board)){
			Search next = {trgt.x,trgt.y-1,SOUTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_4(trgt.x+1,trgt.y,end_z, board)){
			Search next = {trgt.x+1,trgt.y,WEST};
			qu[qu_tail] = next; qu_tail++;
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_4(trgt.x,trgt.y+1,end_z, board)){
			Search next = {trgt.x,trgt.y+1,NORTH};
			qu[qu_tail] = next; qu_tail++;
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_4(trgt.x-1,trgt.y,end_z, board)){
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
#endif

	ap_int<7> now_x = trgt_line->getSinkX();
	ap_int<7> now_y = trgt_line->getSinkY();
	ap_int<8> next_direction_array[4];
#pragma HLS ARRAY_PARTITION variable=next_direction_array complete dim=0
	ap_int<8> next_direction_array_index = 0;
	ap_int<8> next_count, next_id;

	Direction_R trgt_d;


/* ********************************
 * Phase 3-1: シンク層のバックトレース
 ******************************** */

	if(start_z != end_z){

		Point p_s = {now_x, now_y, end_z};
		trgt_line->track[trgt_line->track_index] = p_s; (trgt_line->track_index)++;

#if 0
if (debug_option) { cout << "(" << now_x << "," << now_y << "," << end_z << ")"; }
#endif

		trgt_d = my_board_2[now_y][now_x].d;
		next_direction_array_index = 0;
		if(trgt_d.n) { next_direction_array[next_direction_array_index] = NORTH; next_direction_array_index++; }
		if(trgt_d.e) { next_direction_array[next_direction_array_index] = EAST; next_direction_array_index++; }
		if(trgt_d.s) { next_direction_array[next_direction_array_index] = SOUTH; next_direction_array_index++; }
		if(trgt_d.w) { next_direction_array[next_direction_array_index] = WEST; next_direction_array_index++; }
		next_count = /*(int)*/mt_genrand_int32(0, next_direction_array_index - 1);
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
#pragma HLS LOOP_TRIPCOUNT min=10 max=160 avg=40
////#pragma HLS PIPELINE

			Point p = {now_x, now_y, end_z};
			trgt_line->track[trgt_line->track_index] = p; (trgt_line->track_index)++;

#if 0
if (debug_option) { cout << "(" << now_x << "," << now_y << "," << end_z << ")"; }
#endif

			if(board->box(now_x,now_y,end_z)->isTypeVia()) break;

			trgt_d = my_board_2[now_y][now_x].d;

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


/* ********************************
 * Phase 3-2: ソース層のバックトレース
 ******************************** */

	Point p_v = {now_x, now_y, start_z};
	trgt_line->track[trgt_line->track_index] = p_v; (trgt_line->track_index)++;

#if 0
if (debug_option) { cout << "(" << now_x << "," << now_y << "," << start_z << ")"; }
#endif

	trgt_d = my_board_1[now_y][now_x].d;
	next_direction_array_index = 0;
	if(trgt_d.n) { next_direction_array[next_direction_array_index] = NORTH; next_direction_array_index++; }
	if(trgt_d.e) { next_direction_array[next_direction_array_index] = EAST; next_direction_array_index++; }
	if(trgt_d.s) { next_direction_array[next_direction_array_index] = SOUTH; next_direction_array_index++; }
	if(trgt_d.w) { next_direction_array[next_direction_array_index] = WEST; next_direction_array_index++; }
	next_count = /*(int)*/mt_genrand_int32(0, next_direction_array_index - 1);
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
#pragma HLS LOOP_TRIPCOUNT min=10 max=160 avg=40
////#pragma HLS PIPELINE

		Point p = {now_x, now_y, start_z};
		trgt_line->track[trgt_line->track_index] = p; (trgt_line->track_index)++;

#if 0
if (debug_option) { cout << "(" << now_x << "," << now_y << "," << start_z << ")"; }
#endif

		if(now_x==trgt_line->getSourceX() && now_y==trgt_line->getSourceY()) break;

		trgt_d = my_board_1[now_y][now_x].d;

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

#if 0
if( debug_option ) { cout << endl; }
#endif

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

bool isInserted_3(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board){ // ソース層用
//#pragma HLS INLINE // だめ

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia()) return false;
	if(countLine(x,y,z, board) > 0) return false; // ラインがあるマスは探索しない

	return true;
}

bool isInserted_4(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board){ // シンク層用
//#pragma HLS INLINE // だめ

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;

	Box* trgt_box = board->box(x,y,z);

	if(trgt_box->isTypeInterVia() || trgt_box->isTypeVia()) return false;
	if(countLine(x,y,z, board) > 0) return false; // ラインがあるマスは探索しない

	return true;
}
