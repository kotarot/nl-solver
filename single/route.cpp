/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#include "./main.hpp"
#include "./route.hpp"
#include "./utils.hpp"


extern Board* board;

extern int penalty_T;
extern int penalty_C;

extern vector<int> adj_num;


bool routing(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	trgt_line->clearTrack();
	
	// ソースポートとシンクポートの交換
	trgt_line->changePort();
	
	vector<vector<IntraBox*> > my_board(board->getSizeY(), vector<IntraBox*>(board->getSizeX()));
	IntraBox init = {
		INT_MAX,INT_MAX,INT_MAX,INT_MAX,
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false},
		{false,false,false,false}};
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			my_board[y][x] = new IntraBox;
			*(my_board[y][x]) = init;
		}
	}
	
	int start_x = trgt_line->getSourceX();
	int start_y = trgt_line->getSourceY();
	
	IntraBox* start = my_board[start_y][start_x];
	start->ne = 0; start->nw = 0;
	start->se = 0; start->sw = 0;
	
	queue<Search> qu;
	// 北方向
	if(isInserted(start_x,start_y-1,SOUTH) && isFixed(start_x,start_y,NORTH,-1,trgt_line_id)){
		Search trgt = {start_x,start_y-1,SOUTH,-1};
		qu.push(trgt);
	}
	// 東方向
	if(isInserted(start_x+1,start_y,WEST) && isFixed(start_x,start_y,EAST,-1,trgt_line_id)){
		Search trgt = {start_x+1,start_y,WEST,-1};
		qu.push(trgt);
	}
	// 南方向
	if(isInserted(start_x,start_y+1,NORTH) && isFixed(start_x,start_y,SOUTH,-1,trgt_line_id)){
		Search trgt = {start_x,start_y+1,NORTH,-1};
		qu.push(trgt);
	}
	// 西方向
	if(isInserted(start_x-1,start_y,EAST) && isFixed(start_x,start_y,WEST,-1,trgt_line_id)){
		Search trgt = {start_x-1,start_y,EAST,-1};
		qu.push(trgt);
	}
	
	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		//cout << "(" << trgt.x << "," << trgt.y << ")" << endl;
		
		Box* trgt_box = board->box(trgt.x,trgt.y);
		IntraBox* trgt_ibox = my_board[trgt.y][trgt.x];
		bool update = false;
		
		int turn_count = 0;
		if(trgt.c != trgt.d) turn_count++;
	
		// コスト計算
		if(trgt.d == SOUTH){ // 南から来た
			IntraBox* find_ibox = my_board[trgt.y+1][trgt.x];
			// タッチ数
			int touch_count = countLineNum(trgt.x,trgt.y) - trgt_box->getSouthNum();
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
			IntraBox* find_ibox = my_board[trgt.y][trgt.x-1];
			// タッチ数
			int touch_count = countLineNum(trgt.x,trgt.y) - trgt_box->getWestNum();
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
			IntraBox* find_ibox = my_board[trgt.y-1][trgt.x];
			// タッチ数
			int touch_count = countLineNum(trgt.x,trgt.y) - trgt_box->getNorthNum();
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
			IntraBox* find_ibox = my_board[trgt.y][trgt.x+1];
			// タッチ数
			int touch_count = countLineNum(trgt.x,trgt.y) - trgt_box->getEastNum();
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
		
		// 北方向
		if(trgt.d!=NORTH && isInserted(trgt.x,trgt.y-1,SOUTH) && isFixed(trgt.x,trgt.y,NORTH,trgt.d,trgt_line_id)){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && isInserted(trgt.x+1,trgt.y,WEST) && isFixed(trgt.x,trgt.y,EAST,trgt.d,trgt_line_id)){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && isInserted(trgt.x,trgt.y+1,NORTH) && isFixed(trgt.x,trgt.y,SOUTH,trgt.d,trgt_line_id)){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && isInserted(trgt.x-1,trgt.y,EAST) && isFixed(trgt.x,trgt.y,WEST,trgt.d,trgt_line_id)){
			Search next = {trgt.x-1,trgt.y,EAST,trgt.d};
			qu.push(next);
		}
		
	}
	
	// cout << endl;
	// for(int y=0;y<board->getSizeY();y++){
		// for(int x=0;x<board->getSizeX();x++){
			// IntraBox* trgt_box = my_board[y][x];
			// if(trgt_box->nw > 10000){
				// cout << " +";
			// }
			// else{
				// cout << setw(2) << trgt_box->nw;
			// }
			// if(trgt_box->ne > 10000){
				// cout << " +";
			// }
			// else{
				// cout << setw(2) << trgt_box->ne;
			// }
			// cout << " ";
		// }
		// cout << endl;
		// for(int x=0;x<board->getSizeX();x++){
			// IntraBox* trgt_box = my_board[y][x];
			// if(trgt_box->sw > 10000){
				// cout << " +";
			// }
			// else{
				// cout << setw(2) << trgt_box->sw;
			// }
			// if(trgt_box->se > 10000){
				// cout << " +";
			// }
			// else{
				// cout << setw(2) << trgt_box->se;
			// }
			// cout << " ";
		// }
		// cout << endl;
	// }
	
	
	// バックトレース
	int now_x = trgt_line->getSinkX();
	int now_y = trgt_line->getSinkY();
	int intra_box = -1;
	
	vector<int> adj_cost(8);
	if(now_y!=0){
		adj_cost[0] = my_board[now_y-1][now_x]->sw; // 北，南西
		adj_cost[1] = my_board[now_y-1][now_x]->se; // 北，南東
	}
	else{
		adj_cost[0] = INT_MAX;
		adj_cost[1] = INT_MAX;
	}
	if(now_x!=(board->getSizeX()-1)){
		adj_cost[2] = my_board[now_y][now_x+1]->nw; // 東，北西
		adj_cost[3] = my_board[now_y][now_x+1]->sw; // 東，南西
	}
	else{
		adj_cost[2] = INT_MAX;
		adj_cost[3] = INT_MAX;
	}
	if(now_y!=(board->getSizeY()-1)){
		adj_cost[4] = my_board[now_y+1][now_x]->ne; // 南，北東
		adj_cost[5] = my_board[now_y+1][now_x]->nw; // 南，北西
	}
	else{
		adj_cost[4] = INT_MAX;
		adj_cost[5] = INT_MAX;
	}
	if(now_x!=0){
		adj_cost[6] = my_board[now_y][now_x-1]->se; // 西，南東
		adj_cost[7] = my_board[now_y][now_x-1]->ne; // 西，北東
	}
	else{
		adj_cost[6] = INT_MAX;
		adj_cost[7] = INT_MAX;
	}
	vector<int> min_direction_array;
	int min_cost = INT_MAX, threshold_cost = 10000, min_direction_array_size;
	for (int trying = 0; trying < 5; trying++) {
		for (int a = 0; a < 8; a++) {
			if (adj_cost[a] > threshold_cost) {
				continue;
			}
			if (adj_cost[a] < min_cost) {
				min_direction_array.clear();
				min_direction_array.push_back(a);
				min_cost = adj_cost[a];
			} else if(adj_cost[a] == min_cost){
				min_direction_array.push_back(a);
			}
		}
		min_direction_array_size = (int)(min_direction_array.size());
		if (min_direction_array_size != 0) {
			break;
		}
		threshold_cost *= 10;
	}
	if (min_direction_array_size == 0) {
		return false;
	}
	int adj_count = (int)mt_genrand_int32(0, min_direction_array_size - 1);
	int adj_id = min_direction_array[adj_count];
	//cout << min_cost << endl;
	
	
	//bool p_n = false;
	//bool p_e = false;
	//bool p_s = false;
	//bool p_w = false;
	switch(adj_id){
		case 0: // 北，南西
		now_y = now_y - 1; intra_box = SW;
		//p_n = true;
		break;
		case 1: // 北，南東
		now_y = now_y - 1; intra_box = SE;
		//p_n = true;
		break;
		case 2: // 東，北西
		now_x = now_x + 1; intra_box = NW;
		//p_e = true;
		break;
		case 3: // 東，南西
		now_x = now_x + 1; intra_box = SW;
		//p_e = true;
		break;
		case 4: // 南，北東
		now_y = now_y + 1; intra_box = NE;
		//p_s = true;
		break;
		case 5: // 南，北西
		now_y = now_y + 1; intra_box = NW;
		//p_s = true;
		break;
		case 6: // 西，南東
		now_x = now_x - 1; intra_box = SE;
		//p_w = true;
		break;
		case 7: // 西，北東
		now_x = now_x - 1; intra_box = NE;
		//p_w = true;
		break;
	}
	
	while(now_x!=start_x || now_y!=start_y){
		
		//cout << now_x << "," << now_y << endl;
		Point p = {now_x, now_y};
		trgt_line->pushPointToTrack(p);
		
		Direction trgt_d;
		switch(intra_box){
			case NE:
			trgt_d = my_board[now_y][now_x]->d_ne;
			break;
			case NW:
			trgt_d = my_board[now_y][now_x]->d_nw;
			break;
			case SE:
			trgt_d = my_board[now_y][now_x]->d_se;
			break;
			case SW:
			trgt_d = my_board[now_y][now_x]->d_sw;
			break;
			default:
			assert(!"Undefined Intra-Box");
			break;
		}
		
		vector<int> next_direction_array;
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		//cout << (int)(next_direction_array.size()) << endl;
		
		int next_count = (int)mt_genrand_int32(0, (int)(next_direction_array.size()) - 1);
		int next_id = next_direction_array[next_count];
		
		switch(next_id){
			case NORTH:
			now_y = now_y - 1; // 北へ
			if(intra_box == NE || intra_box == SE) intra_box = SE;
			if(intra_box == NW || intra_box == SW) intra_box = SW;
			//p_n = true; p_e = false; p_s = false; p_w = false;
			break;
			case EAST:
			now_x = now_x + 1; // 東へ
			if(intra_box == NE || intra_box == NW) intra_box = NW;
			if(intra_box == SE || intra_box == SW) intra_box = SW;
			//p_n = false; p_e = true; p_s = false; p_w = false;
			break;
			case SOUTH:
			now_y = now_y + 1; // 南へ
			if(intra_box == NE || intra_box == SE) intra_box = NE;
			if(intra_box == NW || intra_box == SW) intra_box = NW;
			//p_n = false; p_e = false; p_s = true; p_w = false;
			break;
			case WEST:
			now_x = now_x - 1; // 西へ
			if(intra_box == NE || intra_box == NW) intra_box = NE;
			if(intra_box == SE || intra_box == SW) intra_box = SE;
			//p_n = false; p_e = false; p_s = false; p_w = true;
			break;
		}
	}
	
	bool retry = true;
	vector<Point>* trgt_track = trgt_line->getTrack();
	
	while(retry){
		retry = false;
		vector<Point> tmp_track;
		for(int i=0;i<(int)(trgt_track->size());i++){
			tmp_track.push_back((*trgt_track)[i]);
		}
		trgt_track->clear();
		for(int i=0;i<(int)(tmp_track.size());i++){
			if(i>=(int)(tmp_track.size())-2){
				trgt_track->push_back(tmp_track[i]);
				continue;
			}
			if (tmp_track[i + 2].x == tmp_track[i].x && tmp_track[i + 2].y == tmp_track[i].y) {
				retry = true;
				i++;
				continue;
			}
			trgt_track->push_back(tmp_track[i]);
		}
	}
	
	// 表示
	// map<int,map<int,int> > for_print;
	// for(int y=0;y<board->getSizeY();y++){
		// for(int x=0;x<board->getSizeX();x++){
			// Box* trgt_box = board->box(x,y);
			// if(trgt_box->isTypeBlank()){
				// for_print[y][x] = -1;
			// }
			// else{
				// for_print[y][x] = trgt_box->getNumber();
			// }
		// }
	// }
	// for(int i=0;i<(int)(trgt_track->size());i++){
		// int point = (*trgt_track)[i];
		// int point_x = point % board->getSizeX();
		// int point_y = point / board->getSizeX();
		// for_print[point_y][point_x] = -2;
	// }
	// for(int y=0;y<board->getSizeY();y++){
		// for(int x=0;x<board->getSizeX();x++){
			// if(for_print[y][x] == -2) cout << "@";
			// else if(for_print[y][x] == -1) cout << "+";
			// else{
				// if(for_print[y][x] < 10){
					// cout << for_print[y][x];
				// }
				// else{
					// cout << changeIntToChar(for_print[y][x]);
				// }
			// }
		// }
		// cout << endl;
	// }
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			delete my_board[y][x];
		}
	}
	
	return true;
}


bool isInserted(int x,int y,int d){

	if(x<0 || x>(board->getSizeX()-1)) return false;
	if(y<0 || y>(board->getSizeY()-1)) return false;
	
	Box* trgt_box = board->box(x,y);
	
	if(trgt_box->isTypeNumber()) return false;
	else if(trgt_box->isTypeAllFixed()){
		if(d==NORTH){
			return trgt_box->isNorthLineFixed();
		}
		if(d==EAST){
			return trgt_box->isEastLineFixed();
		}
		if(d==SOUTH){
			return trgt_box->isSouthLineFixed();
		}
		if(d==WEST){
			return trgt_box->isWestLineFixed();
		}
	}
	
	return true;
}

bool isFixed(int x,int y,int d,int c,int num){

	Box* trgt_box = board->box(x,y);
	
	if(trgt_box->isTypeNumber() && trgt_box->isTypeAllFixed()){
		if(d==NORTH){
			return trgt_box->isNorthLineFixed();
		}
		if(d==EAST){
			return trgt_box->isEastLineFixed();
		}
		if(d==SOUTH){
			return trgt_box->isSouthLineFixed();
		}
		if(d==WEST){
			return trgt_box->isWestLineFixed();
		}
	}
	if(trgt_box->isTypeBlank() && trgt_box->isTypeAllFixed()){
		if(d==NORTH){
			return trgt_box->isNorthLineFixed();
		}
		if(d==EAST){
			return trgt_box->isEastLineFixed();
		}
		if(d==SOUTH){
			return trgt_box->isSouthLineFixed();
		}
		if(d==WEST){
			return trgt_box->isWestLineFixed();
		}
	}
	if(trgt_box->isTypeBlank() && trgt_box->isTypeHalfFixed()){
		// 半固定マスの線方向と来た方向が一致
		// ＝半固定マスの出口
		if((c==NORTH && trgt_box->isNorthLineFixed())
		|| (c==EAST && trgt_box->isEastLineFixed())
		|| (c==SOUTH && trgt_box->isSouthLineFixed())
		|| (c==WEST && trgt_box->isWestLineFixed())){
			return true;
		}
		else{ // 半固定マスの入口
			int c_num = getConnectedNumber(x,y);
			if(c_num>0 && c_num!=num) return false; // 固定線に接続された数字が対象数と異なる
			setAdjacentNumberProc(x,y);
			for(int i=0;i<(int)(adj_num.size());i++){
				if(adj_num[i]==num) return false; // 固定線が対象数に接する
			}
			if(d==NORTH){
				return trgt_box->isNorthLineFixed();
			}
			if(d==EAST){
				return trgt_box->isEastLineFixed();
			}
			if(d==SOUTH){
				return trgt_box->isSouthLineFixed();
			}
			if(d==WEST){
				return trgt_box->isWestLineFixed();
			}
		}
	}
	
	return true;
}

int countLineNum(int x,int y){

	Box* trgt_box = board->box(x,y);
	
	int count = 0;
	count += trgt_box->getNorthNum();
	count += trgt_box->getEastNum();
	count += trgt_box->getSouthNum();
	count += trgt_box->getWestNum();
	
	return count/2;
}

void recording(int trgt_line_id){
	
	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();
	
	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int new_x = (*trgt_track)[0].x;
	int new_y = (*trgt_track)[0].y;
	
	if(new_x==old_x && new_y==old_y-1){ // 北
		board->box(new_x,new_y)->incrementSouthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		board->box(new_x,new_y)->incrementWestNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		board->box(new_x,new_y)->incrementNorthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		board->box(new_x,new_y)->incrementEastNum();
	}
	
	old_x = new_x; old_y = new_y;
	
	for(int i=1;i<(int)(trgt_track->size());i++){
		new_x = (*trgt_track)[i].x;
		new_y = (*trgt_track)[i].y;
		
		if(new_x==old_x && new_y==old_y-1){ // 北
			board->box(old_x,old_y)->incrementNorthNum();
			board->box(new_x,new_y)->incrementSouthNum();
		}
		else if(new_x==old_x+1 && new_y==old_y){ // 東
			board->box(old_x,old_y)->incrementEastNum();
			board->box(new_x,new_y)->incrementWestNum();
		}
		else if(new_x==old_x && new_y==old_y+1){ // 南
			board->box(old_x,old_y)->incrementSouthNum();
			board->box(new_x,new_y)->incrementNorthNum();
		}
		else if(new_x==old_x-1 && new_y==old_y){ // 西
			board->box(old_x,old_y)->incrementWestNum();
			board->box(new_x,new_y)->incrementEastNum();
		}
		
		old_x = new_x; old_y = new_y;
	}
	
	new_x = trgt_line->getSourceX();
	new_y = trgt_line->getSourceY();
	
	if(new_x==old_x && new_y==old_y-1){ // 北
		board->box(old_x,old_y)->incrementNorthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		board->box(old_x,old_y)->incrementEastNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		board->box(old_x,old_y)->incrementSouthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		board->box(old_x,old_y)->incrementWestNum();
	}
}

void deleting(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	vector<Point>* trgt_track = trgt_line->getTrack();
	
	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int new_x = (*trgt_track)[0].x;
	int new_y = (*trgt_track)[0].y;
	
	if(new_x==old_x && new_y==old_y-1){ // 北
		board->box(new_x,new_y)->decrementSouthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		board->box(new_x,new_y)->decrementWestNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		board->box(new_x,new_y)->decrementNorthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		board->box(new_x,new_y)->decrementEastNum();
	}
	
	old_x = new_x; old_y = new_y;
	
	for(int i=1;i<(int)(trgt_track->size());i++){
		new_x = (*trgt_track)[i].x;
		new_y = (*trgt_track)[i].y;
		
		if(new_x==old_x && new_y==old_y-1){ // 北
			board->box(old_x,old_y)->decrementNorthNum();
			board->box(new_x,new_y)->decrementSouthNum();
		}
		else if(new_x==old_x+1 && new_y==old_y){ // 東
			board->box(old_x,old_y)->decrementEastNum();
			board->box(new_x,new_y)->decrementWestNum();
		}
		else if(new_x==old_x && new_y==old_y+1){ // 南
			board->box(old_x,old_y)->decrementSouthNum();
			board->box(new_x,new_y)->decrementNorthNum();
		}
		else if(new_x==old_x-1 && new_y==old_y){ // 西
			board->box(old_x,old_y)->decrementWestNum();
			board->box(new_x,new_y)->decrementEastNum();
		}
		
		old_x = new_x; old_y = new_y;
	}
	
	new_x = trgt_line->getSourceX();
	new_y = trgt_line->getSourceY();
	
	if(new_x==old_x && new_y==old_y-1){ // 北
		board->box(old_x,old_y)->decrementNorthNum();
	}
	else if(new_x==old_x+1 && new_y==old_y){ // 東
		board->box(old_x,old_y)->decrementEastNum();
	}
	else if(new_x==old_x && new_y==old_y+1){ // 南
		board->box(old_x,old_y)->decrementSouthNum();
	}
	else if(new_x==old_x-1 && new_y==old_y){ // 西
		board->box(old_x,old_y)->decrementWestNum();
	}
}
