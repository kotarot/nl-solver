/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#include "./main.hpp"
#include "./route.hpp"
#include "./utils.hpp"

extern Board* board;

extern int penalty_T;
extern int penalty_C;
extern int penalty_V;

bool routing(int trgt_line_id, bool debug_option){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->clearTrack();
	
	// ボードの初期化
	vector<vector<IntraBox*> > my_board_1(board->getSizeY(), vector<IntraBox*>(board->getSizeX())); // ソース側のボード
	vector<vector<IntraBox*> > my_board_2(board->getSizeY(), vector<IntraBox*>(board->getSizeX())); // シンク側のボード
	IntraBox init = {
		INT_MAX,INT_MAX,INT_MAX,INT_MAX,
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false}};
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			my_board_1[y][x] = new IntraBox;
			*(my_board_1[y][x]) = init;
			my_board_2[y][x] = new IntraBox;
			*(my_board_2[y][x]) = init;
		}
	}
	int start_x, start_y;
	IntraBox* start;
	queue<Search> qu;

	int start_z = trgt_line->getSourceZ();
	int end_z = trgt_line->getSinkZ();


	/*** ソース層の探索 ***/

	// スタート地点の設定
	start_x = trgt_line->getSourceX();
	start_y = trgt_line->getSourceY();
	start = my_board_1[start_y][start_x];
	start->ne = 0; start->nw = 0;
	start->se = 0; start->sw = 0;
	
	// 北方向を探索
	if(isInserted_1(start_x,start_y-1,start_z)){
		Search trgt = {start_x,start_y-1,SOUTH,NOT_USE};
		qu.push(trgt);
	}
	// 東方向を探索
	if(isInserted_1(start_x+1,start_y,start_z)){
		Search trgt = {start_x+1,start_y,WEST,NOT_USE};
		qu.push(trgt);
	}
	// 南方向を探索
	if(isInserted_1(start_x,start_y+1,start_z)){
		Search trgt = {start_x,start_y+1,NORTH,NOT_USE};
		qu.push(trgt);
	}
	// 西方向を探索
	if(isInserted_1(start_x-1,start_y,start_z)){
		Search trgt = {start_x-1,start_y,EAST,NOT_USE};
		qu.push(trgt);
	}
	
	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		Box* trgt_box = board->box(trgt.x,trgt.y,start_z);
		IntraBox* trgt_ibox = my_board_1[trgt.y][trgt.x];
		bool update = false; // コストの更新があったか？
		
		int turn_count = 0;
		if(trgt.c != NOT_USE && trgt.c != trgt.d) turn_count++;
	
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox* find_ibox = my_board_1[trgt.y+1][trgt.x];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z) - trgt_box->getSouthNum();
			if(touch_count < 0){ cout << "error! (error: 10)" << endl; exit(10); }
			// コスト
			int cost_se = (find_ibox->ne) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C + turn_count * BT;
			int cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_1[trgt.y][trgt.x-1];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z) - trgt_box->getWestNum();
			if(touch_count < 0){ cout << "error! (error: 11)" << endl; exit(11); }
			// コスト
			int cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C + turn_count * BT;
			int cost_sw = (find_ibox->se) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_1[trgt.y-1][trgt.x];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z) - trgt_box->getNorthNum();
			if(touch_count < 0){ cout << "error! (error: 12)" << endl; exit(12); }
			// コスト
			int cost_ne = (find_ibox->se) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C + turn_count * BT;
			int cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_1[trgt.y][trgt.x+1];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,start_z) - trgt_box->getEastNum();
			if(touch_count < 0){ cout << "error! (error: 13)" << endl; exit(13); }
			// コスト
			int cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C + turn_count * BT;
			int cost_se = (find_ibox->sw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C + turn_count * BT;
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
		if(trgt.d!=NORTH && isInserted_1(trgt.x,trgt.y-1,start_z)){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_1(trgt.x+1,trgt.y,start_z)){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_1(trgt.x,trgt.y+1,start_z)){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_1(trgt.x-1,trgt.y,start_z)){
			Search next = {trgt.x-1,trgt.y,EAST,trgt.d};
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

            // ソース層にINT_MAXのコスト値が含まれている場合は引き継がない (issue #82)
            if (my_board_1[start_y][start_x]->ne == INT_MAX || my_board_1[start_y][start_x]->nw == INT_MAX ||
                my_board_1[start_y][start_x]->se == INT_MAX || my_board_1[start_y][start_x]->sw == INT_MAX) continue;

			start->ne = my_board_1[start_y][start_x]->ne + trgt_via->getUsedLineNum() * penalty_V;
			start->nw = my_board_1[start_y][start_x]->nw + trgt_via->getUsedLineNum() * penalty_V;
			start->se = my_board_1[start_y][start_x]->se + trgt_via->getUsedLineNum() * penalty_V;
			start->sw = my_board_1[start_y][start_x]->sw + trgt_via->getUsedLineNum() * penalty_V;

			// 北方向を探索
			if(isInserted_2(start_x,start_y-1,end_z)){
				Search trgt = {start_x,start_y-1,SOUTH,NOT_USE};
				qu.push(trgt);
			}
			// 東方向を探索
			if(isInserted_2(start_x+1,start_y,end_z)){
				Search trgt = {start_x+1,start_y,WEST,NOT_USE};
				qu.push(trgt);
			}
			// 南方向を探索
			if(isInserted_2(start_x,start_y+1,end_z)){
				Search trgt = {start_x,start_y+1,NORTH,NOT_USE};
				qu.push(trgt);
			}
			// 西方向を探索
			if(isInserted_2(start_x-1,start_y,end_z)){
				Search trgt = {start_x-1,start_y,EAST,NOT_USE};
				qu.push(trgt);
			}
		}

	}


	/*** シンク層の探索 ***/

	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		Box* trgt_box = board->box(trgt.x,trgt.y,end_z);
		IntraBox* trgt_ibox = my_board_2[trgt.y][trgt.x];
		bool update = false; // コストの更新があったか？
		
		int turn_count = 0;
		if(trgt.c != NOT_USE && trgt.c != trgt.d) turn_count++;
	
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox* find_ibox = my_board_2[trgt.y+1][trgt.x];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z) - trgt_box->getSouthNum();
			if(touch_count < 0){ cout << "error! (error: 10)" << endl; exit(10); }
			// コスト
			int cost_se = (find_ibox->ne) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C + turn_count * BT;
			int cost_sw = (find_ibox->nw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_2[trgt.y][trgt.x-1];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z) - trgt_box->getWestNum();
			if(touch_count < 0){ cout << "error! (error: 11)" << endl; exit(11); }
			// コスト
			int cost_nw = (find_ibox->ne) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_ne = (find_ibox->ne) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C + turn_count * BT;
			int cost_sw = (find_ibox->se) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_2[trgt.y-1][trgt.x];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z) - trgt_box->getNorthNum();
			if(touch_count < 0){ cout << "error! (error: 12)" << endl; exit(12); }
			// コスト
			int cost_ne = (find_ibox->se) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_se = (find_ibox->se) + ML + touch_count * penalty_T + trgt_box->getEastNum() * penalty_C + turn_count * BT;
			int cost_nw = (find_ibox->sw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getWestNum() * penalty_C + turn_count * BT;
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
			IntraBox* find_ibox = my_board_2[trgt.y][trgt.x+1];
			// タッチ数
			int touch_count = countLine(trgt.x,trgt.y,end_z) - trgt_box->getEastNum();
			if(touch_count < 0){ cout << "error! (error: 13)" << endl; exit(13); }
			// コスト
			int cost_ne = (find_ibox->nw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_nw = (find_ibox->nw) + ML + touch_count * penalty_T + trgt_box->getNorthNum() * penalty_C + turn_count * BT;
			int cost_se = (find_ibox->sw) + ML + touch_count * penalty_T + turn_count * BT;
			int cost_sw = (find_ibox->sw) + ML + touch_count * penalty_T + trgt_box->getSouthNum() * penalty_C + turn_count * BT;
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
		if(trgt.d!=NORTH && isInserted_2(trgt.x,trgt.y-1,end_z)){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted_2(trgt.x+1,trgt.y,end_z)){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted_2(trgt.x,trgt.y+1,end_z)){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted_2(trgt.x-1,trgt.y,end_z)){
			Search next = {trgt.x-1,trgt.y,EAST,trgt.d};
			qu.push(next);
		}
	}

if (debug_option) { /*** デバッグ用*/
	cout << endl;
	cout << "LAYER So (line_id: " << trgt_line_id << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox* trgt_box = my_board_1[y][x];
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
			IntraBox* trgt_box = my_board_1[y][x];
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
	cout << "LAYER Si (line_id: " << trgt_line_id << ")" << endl;
	cout << "========" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			IntraBox* trgt_box = my_board_2[y][x];
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
			IntraBox* trgt_box = my_board_2[y][x];
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
} /* ***/

	int now_x = trgt_line->getSinkX();
	int now_y = trgt_line->getSinkY();
	int intra_box = -1;
//	int threshold_cost = 1000000000;
	vector<int> next_direction_array;
	int next_count, next_id;

	/*** シンク層のバックトレース ***/

	if(start_z != end_z){

		intra_box = NE;

		while(1){

			Point p = {now_x, now_y, end_z};
			trgt_line->pushPointToTrack(p);
// デバッグ用			cout << "(" << now_x << "," << now_y << "," << end_z << ")";

			if(board->box(now_x,now_y,end_z)->isTypeVia()) break;

			Direction trgt_d;
			switch(intra_box){
				case NE:
				trgt_d = my_board_2[now_y][now_x]->d_ne; break;

				case NW:
				trgt_d = my_board_2[now_y][now_x]->d_nw; break;

				case SE:
				trgt_d = my_board_2[now_y][now_x]->d_se; break;

				case SW:
				trgt_d = my_board_2[now_y][now_x]->d_sw; break;

				default:
				assert(!"Undefined Intra-Box"); break;
			}

			next_direction_array.clear();
			if(trgt_d.n) next_direction_array.push_back(NORTH);
			if(trgt_d.e) next_direction_array.push_back(EAST);
			if(trgt_d.s) next_direction_array.push_back(SOUTH);
			if(trgt_d.w) next_direction_array.push_back(WEST);
			next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
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

	while(1){

		Point p = {now_x, now_y, start_z};
		trgt_line->pushPointToTrack(p);
// デバッグ用		cout << "(" << now_x << "," << now_y << "," << start_z << ")";

		if(now_x==trgt_line->getSourceX() && now_y==trgt_line->getSourceY()) break;

		Direction trgt_d;
		switch(intra_box){
			case NE:
			trgt_d = my_board_1[now_y][now_x]->d_ne; break;

			case NW:
			trgt_d = my_board_1[now_y][now_x]->d_nw; break;

			case SE:
			trgt_d = my_board_1[now_y][now_x]->d_se; break;

			case SW:
			trgt_d = my_board_1[now_y][now_x]->d_sw; break;

			default:
			assert(!"Undefined Intra-Box"); break;
		}

		next_direction_array.clear();
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
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
// デバッグ用	cout << endl;
	

	/*** ターゲットラインのトラックを整理 ***/

	vector<Point>* trgt_track = trgt_line->getTrack();
	bool retry = true;
	while(retry){
		retry = false;

		// トラックを一時退避
		vector<Point> tmp_track;
		for (int i = 0; i < (int)(trgt_track->size()); i++) {
			tmp_track.push_back((*trgt_track)[i]);
		}

		// 冗長部分を排除してトラックを整理
		trgt_track->clear();
		for (int i = 0; i < (int)(tmp_track.size()); i++) {
			if ((int)(tmp_track.size()) - 2 <= i) {
				trgt_track->push_back(tmp_track[i]);
				continue;
			}
			if (tmp_track[i].x == tmp_track[i + 2].x && tmp_track[i].y == tmp_track[i + 2].y && tmp_track[i].z == tmp_track[i+2].z) {
				retry = true;
				i++;
				continue;
			}
			trgt_track->push_back(tmp_track[i]);
		}
	}
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			delete my_board_1[y][x];
			delete my_board_2[y][x];
		}
	}
	
	return true;
}

bool isInserted_1(int x,int y,int z){ // ソース層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;
	
	Box* trgt_box = board->box(x,y,z);
	
	if(trgt_box->isTypeInterVia()) return false;
	
	return true;
}

bool isInserted_2(int x,int y,int z){ // シンク層用

	// 盤面の端
	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;
	
	Box* trgt_box = board->box(x,y,z);
	
	if(trgt_box->isTypeInterVia() || trgt_box->isTypeVia()) return false;
	
	return true;
}

int countLine(int x,int y,int z){

	Box* trgt_box = board->box(x,y,z);
	
	int count = 0;
	count += trgt_box->getNorthNum();
	count += trgt_box->getEastNum();
	count += trgt_box->getSouthNum();
	count += trgt_box->getWestNum();
	
	return count/2;
}

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
