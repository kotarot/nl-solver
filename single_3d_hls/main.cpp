/**
 * main.cpp
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


// ================================ //
// メルセンヌ・ツイスタ
// ================================ //
#include "mt19937ar.hpp"

void mt_init_genrand(unsigned long s) {
	init_genrand(s);
}

// AからBの範囲の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
	return genrand_int32() % (b - a + 1) + a;
}


bool nlsolver(ap_int<8> boardmat[MAX_LAYER][MAX_BOXES][MAX_BOXES], ap_int<8> *status) {
#pragma HLS INTERFACE s_axilite port=boardmat bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=status bundle=AXI4LS
#pragma HLS INTERFACE s_axilite port=return bundle=AXI4LS

	ap_int<32> outer_loops = O_LOOP;  // 外ループ回数

	//int line_to_viaid[100];			// Lineと対応するViaの内部ID対応
	//int line_to_via_priority[100];	// Lineに対応するViaの優先度(*数字が高いほど重要！)
	//int via_priority;				// 採用するViaの優先度

	ap_int<16> penalty_T; // penalty of "touch"
	ap_int<16> penalty_C; // penalty of "cross"
	ap_int<16> penalty_V; // penalty of "via duplication"

	Board boardobj;
	initialize(boardmat, &boardobj); // 問題盤の生成
	Board *board = &boardobj;

//if( print_option ) { printBoard(); }

	// 乱数の初期化
	mt_init_genrand(12345); //mt_init_genrand((unsigned long)time(NULL));
	// ペナルティの初期化
	penalty_T = 0;
	penalty_C = 0;
	penalty_V = 0;

	ap_int<8> output;

	// 初期ルーティング
	for (ap_int<8> i = 1; i <= board->getLineNum(); i++) {
		// 数字が隣接する場合スキップ
		if(board->line(i)->getHasLine() == false) continue;

		if( !routing(i, penalty_T, penalty_C, penalty_V, board, &output) ){
			//cerr << "Cannot solve!! (error: 1)" << endl;
			//exit(1);
			*status = 1; return false; 
		}
	}
	for (ap_int<8> i = 1; i<= board->getLineNum(); i++) {
		// 数字が隣接する場合スキップ
		if(board->line(i)->getHasLine() == false) continue;

		recordLine(i, board);
	}

	//via_priority = 0;

	// 探索スタート!!
	// 外ループ
	for (ap_int<32> m = 2; m <= outer_loops + 1; m++) {

		// 解導出フラグ
		bool complete = false;

//		if (m / 10 > via_priority){
//			via_priority++;
//if( print_option ) cout << "Via priority is " << via_priority << endl;
//		}

		// 内ループ
		for (ap_int<32> n = 1; n <= I_LOOP; n++) { // 内ループ

			//cout << m-1 << ":" << n << endl;

			// 問題中で数字が隣接していないラインを選択
			ap_int<8> id;
			do {
				id = mt_genrand_int32(1, board->getLineNum());
			} while (board->line(id)->getHasLine() == false);

			// 経路の削除
			deleteLine(id, board);

			// ペナルティの設定
			penalty_T = NT * (mt_genrand_int32(0, m - 1)); //penalty_T = (int)(NT * (mt_genrand_int32(0, m - 1)));
			penalty_C = NC * (mt_genrand_int32(0, m - 1)); //penalty_C = (int)(NC * (mt_genrand_int32(0, m - 1)));
			penalty_V = NV * (mt_genrand_int32(0, m - 1)); //penalty_V = (int)(NV * (mt_genrand_int32(0, m - 1)));

			// ビア指定
			// int via_idx = 1; // ビア番号
			// board->line(id)->setSpecifiedVia(via_idx);
			// ビア指定解除
			// board->line(id)->setSpecifiedVia(NOT_USE);

			// Via指定
			//if(line_to_viaid[id] != 0){
			//	if(line_to_via_priority[id] >= via_priority)
			//		board->line(id)->setSpecifiedVia(line_to_viaid[id]);
			//	else
			//		board->line(id)->setSpecifiedVia(NOT_USE);
			//}

			// 経路の探索
			if ( !routing(id, penalty_T, penalty_C, penalty_V, board, &output) ) {
				//cerr << "Cannot solve!! (error: 2)" << endl; // 失敗したらプログラム終了
				//exit(2);
				*status = 2; return false;
			}
			// 経路の記録
			recordLine(id, board);

			// 終了判定（解導出できた場合，正解を出力）
			if(isFinished(board)){

#if 0
				// 最終ルーティング
				for (ap_int<8> i = 1; i <= board->getLineNum(); i++) {
					// 数字が隣接する場合スキップ
					if(board->line(i)->getHasLine() == false) continue;
		
					// 経路の削除
					deleteLine(i);

					if( !final_routing(i, board) ){
						//cerr << "Cannot solve!! (error: 3)" << endl;
						//exit(3);
						*status = 3; return false;
					}

					// 経路の記録
					recordLine(i, board);
				}
#endif

				//finish_time = clock();

//if( print_option ) { printSolution(); }
				generateSolution(boardmat, board);

#if 0
				if (out_filename != NULL) {
					printSolutionToFile(out_filename);
					cout << "--> Saved to " << out_filename << endl << endl;
				}

				cout << "SUMMARY" << endl;
				cout << "-------" << endl;
				cout << " - filename:   " << in_filename << endl;
				cout << " - size:       " << board->getSizeX() << " x " << board->getSizeY() << " x " << board->getSizeZ() << endl;
				cout << " - iterations: " << (m - 1) << endl;
				cout << " - CPU time:   "
				     << ((double)(finish_time - start_time) / (double)CLOCKS_PER_SEC)
				     << " sec" << endl;
#endif

				complete = true;
				break;
			}
		}
		if(complete) break; // 正解出力後は外ループも脱出
	}
	
	
	// 解導出できなかった場合
	if(!isFinished(board)){
		//finish_time = clock();
		
// 現状の結果を出力
//if( print_option ) {
//	for(int i=1;i<=board->getLineNum();i++){
//		printLine(i);
//	}
//	cout << endl;
//}

#if 0
		cout << "SUMMARY" << endl;
		cout << "-------" << endl;
		cout << " - filename:   " << in_filename << endl;
		cout << " - size:       " << board->getSizeX() << " x " << board->getSizeY() << " x " << board->getSizeZ() << endl;
		cout << " - iterations: " << outer_loops << endl;
		cout << " - CPU time:   "
		     << ((double)(finish_time - start_time) / (double)CLOCKS_PER_SEC)
		     << " sec" << endl;
#endif

		//cerr << "Cannot solve!! (error: 4)" << endl;
		//exit(4);
		*status = 4; return false;
	}

	//delete board;
	return true;
}

/**
 * 問題盤の初期化 (HLS ver.)
 * @args: boardmat, board
 */
void initialize(ap_int<8> boardmat[MAX_LAYER][MAX_BOXES][MAX_BOXES], Board *board) {

	// line_to_viaの初期化
//	for (ap_int<8> i = 0; i < 100; i++) {
//#pragma HLS PIPELINE
//		line_to_viaid[i] = 0;
//	}

	ap_int<7> size_x = 8, size_y = 5; ap_int<5> size_z = 2;
	ap_int<8> line_num = 2;
	ap_int<8> via_num = 1;
	//map<int,int> lx_0, ly_0, lz_0, lx_1, ly_1, lz_1;
	//map<int,int> vx_0, vy_0, vz_0, vx_1, vy_1, vz_1;
	//map<int,bool> adjacents; // 初期状態で数字が隣接している
	ap_int<7> lx_0[MAX_LINES]; ap_int<7> lx_1[MAX_LINES];
	ap_int<7> ly_0[MAX_LINES]; ap_int<7> ly_1[MAX_LINES];
	ap_int<5> lz_0[MAX_LINES]; ap_int<5> lz_1[MAX_LINES];
	ap_int<7> vx_0[MAX_VIAS]; ap_int<7> vx_1[MAX_VIAS];
	ap_int<7> vy_0[MAX_VIAS]; ap_int<7> vy_1[MAX_VIAS];
	ap_int<5> vz_0[MAX_VIAS]; ap_int<5> vz_1[MAX_VIAS];
	bool adjacents[MAX_LINES];
	bool appear_line[MAX_LINES];
	bool appear_via[MAX_VIAS];

	for (ap_int<8> i = 0; i < MAX_LINES; i++) {
#pragma HLS PIPELINE
		appear_line[i] = false;
	}
	for (ap_int<8> i = 0; i < MAX_VIAS; i++) {
#pragma HLS PIPELINE
		appear_via[i] = false;
	}
	for (ap_int<5> z = 0; z < MAX_LAYER; z++) {
		for (ap_int<7> y = 0; y < MAX_BOXES; y++) {
			for (ap_int<7> x = 0; x < MAX_BOXES; x++) {
#pragma HLS PIPELINE
				ap_int<8> val = boardmat[z][y][x];
				// LINE
				if (0 < val && val < 100) {
					if (appear_line[val] == false) {
						appear_line[val] = true;
						lx_0[val] = x; ly_0[val] = y; lz_0[val] = z;
					} else {
						lx_1[val] = x; ly_1[val] = y; lz_1[val] = z;
					}
				}
				// VIA
				else if (val == 100) {
					if (appear_via[1] == false) {
						appear_via[1] = true;
						vx_0[1] = x; vy_0[1] = y; vz_0[1] = z;
					} else {
						vx_1[1] = x; vy_1[1] = y; vz_1[1] = z;
					}
				}
			}
		}
	}

	board->init(size_x, size_y, size_z, line_num, via_num);

	for (ap_int<8> i = 1; i <= line_num; i++) {
		Box* trgt_box_0 = board->box(lx_0[i],ly_0[i],lz_0[i]);
		Box* trgt_box_1 = board->box(lx_1[i],ly_1[i],lz_1[i]);
		trgt_box_0->setTypeNumber();
		trgt_box_1->setTypeNumber();
		trgt_box_0->setIndex(i);
		trgt_box_1->setIndex(i);
		Line* trgt_line = board->line(i);
		trgt_line->setSourcePort(lx_0[i],ly_0[i],lz_0[i]);
		trgt_line->setSinkPort(lx_1[i],ly_1[i],lz_1[i]);
		if(trgt_line->getSourceZ() > trgt_line->getSinkZ()){
			trgt_line->changePort();
		}
		trgt_line->setHasLine(!adjacents[i]);
	}
	for (ap_int<8> i = 1; i <= via_num; i++) {
		Box* trgt_box_0 = board->box(vx_0[i],vy_0[i],vz_0[i]);
		Box* trgt_box_1 = board->box(vx_1[i],vy_1[i],vz_1[i]);
		trgt_box_0->setTypeVia();
		trgt_box_1->setTypeVia();
		trgt_box_0->setIndex(i);
		trgt_box_1->setIndex(i);
		Via* trgt_via = board->via(i);
		trgt_via->setSourcePort(vx_0[i],vy_0[i],vz_0[i]);
		trgt_via->setSinkPort(vx_1[i],vy_1[i],vz_1[i]);
		if(trgt_via->getSourceZ() > trgt_via->getSinkZ()){
			trgt_via->changePort();
		}
		for (ap_int<5> z = trgt_via->getSourceZ() + 1; z < trgt_via->getSinkZ(); z++) {
#pragma HLS PIPELINE
			Box* trgt_box_2 = board->box(vx_0[i],vy_0[i],z);
			trgt_box_2->setTypeInterVia();
			trgt_box_2->setIndex(i);
		}
	}
	
	for (ap_int<5> z = 0; z < size_z; z++) {
		for (ap_int<7> y = 0; y < size_y; y++) {
			for (ap_int<7> x = 0; x < size_x; x++) {
#pragma HLS PIPELINE
				Box* trgt_box = board->box(x,y,z);
				if(!(trgt_box->isTypeNumber() || trgt_box->isTypeVia() || trgt_box->isTypeInterVia())) trgt_box->setTypeBlank();
			}
		}
	}
}

#if 0
/**
 * 問題盤の初期化
 * @args: 問題ファイル名
 */
void initialize(char* filename){

	ifstream ifs(filename);
	string str;
	
	if(ifs.fail()){
		cerr << "Problem file does not exist." << endl;
		exit(-1);
	}

	// line_to_viaの初期化
	for(int i=0; i<100; i++) line_to_viaid[i] = 0;
	
	int size_x, size_y, size_z;
	int line_num;
	int via_num = 0;
	map<int,int> lx_0, ly_0, lz_0, lx_1, ly_1, lz_1;
	map<int,int> vx_0, vy_0, vz_0, vx_1, vy_1, vz_1;
	map<int,bool> adjacents; // 初期状態で数字が隣接している

	while(getline(ifs,str)){
		if(str.at(0) == '#') continue;
		else if(str.at(0) == 'S'){ // 盤面サイズの読み込み
			str.replace(str.find("S"),5,"");
			str.replace(str.find("X"),1," ");
			str.replace(str.find("X"),1," ");
			istringstream is(str);
			is >> size_x >> size_y >> size_z;
		}
		else if(str.at(0) == 'L' && str.at(4) == '_'){ // ライン個数の読み込み
			str.replace(str.find("L"),9,"");
			istringstream is(str);
			is >> line_num;
		}
		else if(str.at(0) == 'L' && str.at(4) == '#'){ // ライン情報の読み込み
			str.replace(str.find("L"),5,"");
			int index;
			while((index=str.find("("))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(")"))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(","))!=-1){
				str.replace(index,1," ");
			}
			int i, a, b, c, d, e, f;
			istringstream is(str);
			is >> i >> a >> b >> c >> d >> e >> f;
			lx_0[i] = a; ly_0[i] = b; lz_0[i] = c-1; lx_1[i] = d; ly_1[i] = e; lz_1[i] = f-1;

			// ViaIDが割り当てられている場合は辞書に追加
			if(!is.eof()){
				int vid, vp;
				is >> vid >> vp;
				if(vid>=0 && vid<100){
					line_to_viaid[i] = vid;
					line_to_via_priority[i] = vp;
				}
			}

			// 初期状態で数字が隣接しているか判断
			int dx = lx_0[i] - lx_1[i];
			int dy = ly_0[i] - ly_1[i];
			int dz = lz_0[i] - lz_1[i];
			if ((dz == 0 && dx == 0 && (dy == 1 || dy == -1)) || (dz == 0 && (dx == 1 || dx == -1) && dy == 0)) {
				adjacents[i] = true;
			} else {
				adjacents[i] = false;
			}
		}
		else if(str.at(0) == 'V'){ // ビア情報の読み込み
			via_num++;
			str.replace(str.find("V"),4,"");
			int index;
			while((index=str.find("("))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(")"))!=-1){
				str.replace(index,1,"");
			}
			while((index=str.find(","))!=-1){
				str.replace(index,1," ");
			}
			string s;
			int i, a, b, c, d, e, f;
			i = via_num;
			istringstream is(str);
			is >> s	>> a >> b >> c >> d >> e >> f;

			while(!is.eof()){
				is >> d >> e >> f;
			}
			vx_0[i] = a; vy_0[i] = b; vz_0[i] = c-1; vx_1[i] = d; vy_1[i] = e; vz_1[i] = f-1;
		}
		else continue;
	}

	board = new Board(size_x,size_y,size_z,line_num,via_num);
	
	for(int i=1;i<=line_num;i++){
		Box* trgt_box_0 = board->box(lx_0[i],ly_0[i],lz_0[i]);
		Box* trgt_box_1 = board->box(lx_1[i],ly_1[i],lz_1[i]);
		trgt_box_0->setTypeNumber();
		trgt_box_1->setTypeNumber();
		trgt_box_0->setIndex(i);
		trgt_box_1->setIndex(i);
		Line* trgt_line = board->line(i);
		trgt_line->setSourcePort(lx_0[i],ly_0[i],lz_0[i]);
		trgt_line->setSinkPort(lx_1[i],ly_1[i],lz_1[i]);
		if(trgt_line->getSourceZ() > trgt_line->getSinkZ()){
			trgt_line->changePort();
		}
		trgt_line->setHasLine(!adjacents[i]);
	}
	for(int i=1;i<=via_num;i++){
		Box* trgt_box_0 = board->box(vx_0[i],vy_0[i],vz_0[i]);
		Box* trgt_box_1 = board->box(vx_1[i],vy_1[i],vz_1[i]);
		trgt_box_0->setTypeVia();
		trgt_box_1->setTypeVia();
		trgt_box_0->setIndex(i);
		trgt_box_1->setIndex(i);
		Via* trgt_via = board->via(i);
		trgt_via->setSourcePort(vx_0[i],vy_0[i],vz_0[i]);
		trgt_via->setSinkPort(vx_1[i],vy_1[i],vz_1[i]);
		if(trgt_via->getSourceZ() > trgt_via->getSinkZ()){
			trgt_via->changePort();
		}
		for(int z=trgt_via->getSourceZ()+1;z<trgt_via->getSinkZ();z++){
			Box* trgt_box_2 = board->box(vx_0[i],vy_0[i],z);
			trgt_box_2->setTypeInterVia();
			trgt_box_2->setIndex(i);
		}
	}
	
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

bool isFinished(Board *board) {

	bool for_check[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	for (ap_int<5> z = 0; z < board->getSizeZ(); z++) {
		for (ap_int<7> y = 0; y < board->getSizeY(); y++){
			for (ap_int<7> x = 0; x < board->getSizeX(); x++) {
#pragma HLS PIPELINE
				for_check[z][y][x] = false;
			}
		}
	}
	for (ap_int<8> i = 1; i <= board->getLineNum(); i++) {
		Line* trgt_line = board->line(i);
		for (ap_int<16> j = 0; j < trgt_line->track_index; j++) {
#pragma HLS PIPELINE
			Point p = (trgt_line->track)[j];
			ap_int<7> point_x = p.x;
			ap_int<7> point_y = p.y;
			ap_int<5> point_z = p.z;
			if(for_check[point_z][point_y][point_x]){ return false; }
			else{ for_check[point_z][point_y][point_x] = true; }
		}
	}
	
	return true;
}

// 解ボードを生成
void generateSolution(ap_int<8> boardmat[MAX_LAYER][MAX_BOXES][MAX_BOXES], Board *board) {

	for (ap_int<5> z = 0; z < board->getSizeZ(); z++) {
		for (ap_int<7> y = 0; y < board->getSizeY(); y++) {
			for (ap_int<7> x = 0; x < board->getSizeX(); x++) {
				boardmat[z][y][x] = 0;
			}
		}
	}
	for (ap_int<8> i = 1; i <= board->getLineNum(); i++) {
		Line* trgt_line = board->line(i);
		if(!trgt_line->getHasLine()){
			boardmat[trgt_line->getSourceZ()][trgt_line->getSourceY()][trgt_line->getSourceX()] = i;
			boardmat[trgt_line->getSinkZ()][trgt_line->getSinkY()][trgt_line->getSinkX()] = i;
			continue;
		}
		for (ap_int<8> j = 0; j < trgt_line->track_index; j++) {
			Point p = (trgt_line->track)[j];
			int point_x = p.x;
			int point_y = p.y;
			int point_z = p.z;
			boardmat[point_z][point_y][point_x] = i;
		}
	}
	for (ap_int<8> i = 1; i <= board->getViaNum(); i++) {
		Via* trgt_via = board->via(i);
		int via_x = trgt_via->getSourceX();
		int via_y = trgt_via->getSourceY();
		int via_z = trgt_via->getSourceZ();
		int line_num = boardmat[via_z][via_y][via_x]; 
		for (ap_int<5> z = via_z + 1; z < trgt_via->getSinkZ(); z++) {
			boardmat[z][via_y][via_x] = line_num;
		}
	}

#if 0
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
					cout << " \033[37m00\033[0m";
				}else{
					Box* trgt_box = board->box(x,y,z);
					if(trgt_box->isTypeNumber()){
						// 線が引かれているマス [端点] (2桁表示)
						cout << " " << getcolorescape_fore(n) << setfill('0') << setw(2) << n << "\033[0m";
					}else{
						// 線が引かれているマス [非端点] (2桁表示)
						cout << " " << getcolorescape(n) << setfill('0') << setw(2) << n << "\033[0m";
					}
				}
			}
			cout << endl;
		}
		cout << endl;
	}
#endif

}
