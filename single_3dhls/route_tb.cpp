/**
 * route_tb.cpp
 *
 * for Vivado HLS
 */

//#include <stdio.h>
//#include <string.h>
//#include <ap_int.h>

#include "main.hpp"

Board* board; // 対象ボード

int penalty_T; // penalty of "touch"
int penalty_C; // penalty of "cross"
int penalty_V; // penalty of "via duplication"

//bool routing(int trgt_line_id, Board *_board, int penalty_T, int penalty_C, int penalty_V);
bool routing(int trgt_line_id, int penalty_T, int penalty_C, int penalty_V, int *output);

#if 0
/**
 * 問題盤の初期化 (テスト用)
 */
void initialize_test(){

	int size_x = 5, size_y = 5, size_z = 1;
	int line_num = 1;
	int via_num = 0;
	//map<int,int> lx_0, ly_0, lz_0, lx_1, ly_1, lz_1;
	//map<int,int> vx_0, vy_0, vz_0, vx_1, vy_1, vz_1;
	//map<int,bool> adjacents; // 初期状態で数字が隣接している

	board = new Board(5, 5, 1, 1, 0);

	Box* trgt_box_0 = board->box(0, 0, 0);
	Box* trgt_box_1 = board->box(3, 3, 0);
	trgt_box_0->setTypeNumber();
	trgt_box_1->setTypeNumber();
	trgt_box_0->setIndex(1);
	trgt_box_1->setIndex(1);
	Line* trgt_line = board->line(1);
	trgt_line->setSourcePort(0, 0, 0);
	trgt_line->setSinkPort(3, 3, 0);

	for(int z=0;z<size_z;z++){
		for(int y=0;y<size_y;y++){
			for(int x=0;x<size_x;x++){
				Box* trgt_box = board->box(x,y,z);
				if(!(trgt_box->isTypeNumber() || trgt_box->isTypeVia() || trgt_box->isTypeInterVia())) trgt_box->setTypeBlank();
			}
		}
	}
}
#endif

int main() {
	using namespace std;

	//initialize_test();

	int status;
	//bool result = routing(1, board, 1, 1, 1);
	bool result = routing(1, 1, 1, 1, &status);
	cout << "result = " << result << endl;
	cout << "status = " << status << endl;

	return 0;
}
