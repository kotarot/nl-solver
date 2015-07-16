#include "./main.hpp"

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


int main(int argc, char *argv[]){
	
	// ファイルの読み込み
	if(argc != 3){
		cout << "Usage: ./solver.exe InputFile Fixed" << endl;
		return 0;
	}
	initialize(argv[1]); // 問題盤の生成
	printBoard(); // 問題盤の表示
	
	// 固定フラグの生成
	int fixed = atoi(argv[2]);
	if(fixed==1){
		generateFixFlag();
	}
	
	// 乱数の初期化
	srand((unsigned int)time(NULL));
	// ペナルティの初期化
	penalty_T = 0;
	penalty_C = 0;
	
	// 初期ルーティング
	for(int i=1;i<=board->getLineNum();i++){
		if(!routing(i)){
			cout << "Cannot solve!!" << endl;
			return 0;
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		recording(i);
	}
	
	// 中間ポートを利用するか？
	// 利用している間，ライン不通過回数をカウントしない
	bool use_intermediate_port = false;
	// 中間ポートに設定するマスとそれを利用する数字
	int inter_x, inter_y, inter_line;
	
	
	// 探索スタート!!
	for(int m=2;m<=O_LOOP;m++){ // 外ループ
	
		if(!use_intermediate_port){ // 中間ポートを利用しない場合
			cout << "loop " << (m-1) << endl;
			if(m>INIT){ resetCandidate(); }
		}
		else{ // 中間ポートを利用する場合
			cout << "loop " << (m-1) << "+" << endl;
		}
		
		// 解導出フラグ
		bool complete = false;
		
		for(int n=1;n<=I_LOOP;n++){ // 内ループ
			if(m>INIT && !use_intermediate_port){ checkLineNonPassed(); }
			int id = rand() % board->getLineNum() + 1;
			//cout << "(" << m << "," << n << ")Re-route Line" << id << endl;
			
			// 経路の削除
			deleting(id);
			
			// ペナルティの設定
			penalty_T = (int)(NT * (rand() % m));
			penalty_C = (int)(NC * (rand() % m));
			
			// 経路の探索
			bool success = false;
			int count = 1;
			if((board->line(id))->isIntermediateUsed()){ // 中間ポート利用経路探索
				while(!success){
					if(++count>10) break; // 10回失敗したら諦める
					if(!(success = routingSourceToI(id))){
						continue;
					}
					success = routingIToSink(id);
				}
			}
			else{ // 通常経路探索
				success = routing(id);
				if(!success){
					cout << "Cannot solve!!" << endl; // 失敗したらプログラム終了
					return 0;
				}
			}
			
			// 経路の記録
			// 中間ポート利用に失敗した場合，通常経路探索した後に内ループ脱出
			if(count>10){ // 通常経路探索（中間ポート利用に失敗）
				success = routing(id);
				if(!success){
					cout << "Cannot solve!!" << endl; // 失敗したらプログラム終了
					return 0;
				}
				recording(id);
				break;
			}
			recording(id);
		
			// 終了判定（解導出できた場合，正解を出力）
			if(isFinished()){
				printSolution();
				complete = true;
				break;
			}
		}
		if(complete) break; // 正解出力後は外ループも脱出
		
		// 中間ポートを使用した次のループでは，中間ポートを利用しない
		if(use_intermediate_port){
			use_intermediate_port = false;
			board->line(inter_line)->setIntermediateUnuse();
			continue;
		}
		
		// 不通過マスの調査->中間ポートを利用するか？
		int candidate_count = 0; // 候補数
		inter_x = -1;
		inter_y = -1;
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				Box* trgt_box = board->box(x,y);
				if(trgt_box->isCandidate()){
					candidate_count++;
					cout << "(" << x << "," << y << ")"; // 不通過マス
				}
			}
		}
		if(candidate_count==0) continue; // 候補数0なら利用しない
		cout << endl;
		
		// 候補の中から中間ポートに設定するマスをランダムに選択
		int c_d = rand() % candidate_count; // 選択は候補の中で何番目か？
		int n_d = 0; // 何番目なのかをカウントする用の変数
		
		bool flag = false; // 二重ループのためフラグが必要
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				Box* trgt_box = board->box(x,y);
				if(!trgt_box->isCandidate()){
					continue;
				}
				if(n_d==c_d){
					flag = true;
					inter_x = x; inter_y = y;
					break;
				}
				n_d++;
			}
			if(flag) break;
		}
		
		// 中間ポートの四方を見て，中間ポートを利用する数字をランダムに選択
		checkCandidateLine(inter_x,inter_y); // 候補となるラインを調査
		
		candidate_count = 0; // 候補数
		inter_line = -1;
		for(int i=1;i<=board->getLineNum();i++){
			Line* trgt_line = board->line(i);
			if(trgt_line->isCandidate()){
				candidate_count++;
				if(candidate_count>1) cout << ", ";
				cout << i;
			}
		}
		cout << endl;
		
		c_d = rand() % candidate_count; // 選択は候補の中で何番目か？
		n_d = 0; // 何番目なのかをカウントする用の変数
		
		for(int i=1;i<=board->getLineNum();i++){
			Line* trgt_line = board->line(i);
			if(!trgt_line->isCandidate()) continue;
			if(n_d==c_d){
				inter_line = i;
				break;
			}
			n_d++;
		}
		
		cout << "Set (" << inter_x << "," << inter_y << ") InterPort of Line " << inter_line << endl;
		Line* line_i = board->line(inter_line);
		line_i->setIntermediateUse();
		line_i->setIntermediatePort(inter_x,inter_y);
		use_intermediate_port = true;
		// m--; // 使うかどうか思案中・・・
		
		
		// ペナルティ更新（旧）
		//penalty_T = (int)(NT * m);
		//penalty_C = (int)(NC * m);
	}
	
	
	// 解導出できなかった場合
	if(!isFinished()){
		for(int i=1;i<=board->getLineNum();i++){
			printLine(i);
		}
	}
	
	//デバッグ用
	//for(int y=0;y<board->getSizeY();y++){
		//for(int x=0;x<board->getSizeX();x++){
			//cout << "(" << x << "," << y << ") ";
			//Box* trgt_box = board->box(x,y);
			//cout << " N:" << trgt_box->getNorthNum();
			//cout << " E:" << trgt_box->getEastNum();
			//cout << " S:" << trgt_box->getSouthNum();
			//cout << " W:" << trgt_box->getWestNum();
			//cout << endl;
		//}
	//}
	
	//for(int i=1;i<=board->getLineNum();i++){
	//	calcCost(i);
	//}
	
	delete board;
	return 0;
}

void initialize(char* filename){

	ifstream ifs(filename);
	string str;
	
	if(ifs.fail()){
		cerr << "File do not exist.\n";
		exit(1);
	}
	
	int size_x, size_y;
	int line_num;
	map<int,int> x_0, y_0, x_1, y_1;

	while(getline(ifs,str)){
		if(str.at(0) == '#') continue;
		else if(str.at(0) == 'S'){ // 盤面サイズの読み込み
			str.replace(str.find("S"),5,"");
			str.replace(str.find("X"),1," ");
			istringstream is(str);
			is >> size_x >> size_y;
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
			str.replace(str.find("-"),1," ");
			int i, a, b, c, d;
			istringstream is(str);
			is >> i >> a >> b >> c >> d;
			x_0[i] = a; y_0[i] = b; x_1[i] = c; y_1[i] = d;
		}
		else continue;
	}
	
	board = new Board(size_x,size_y,line_num);
	
	for(int i=1;i<=line_num;i++){
		Box* trgt_box_0 = board->box(x_0[i],y_0[i]);
		Box* trgt_box_1 = board->box(x_1[i],y_1[i]);
		trgt_box_0->setTypeNumber();
		trgt_box_1->setTypeNumber();
		trgt_box_0->setNumber(i);
		trgt_box_1->setNumber(i);
		Line* trgt_line = board->line(i);
		trgt_line->setSourcePort(x_0[i],y_0[i]);
		trgt_line->setSinkPort(x_1[i],y_1[i]);
	}
	
	for(int y=0;y<size_y;y++){
		for(int x=0;x<size_x;x++){
			Box* trgt_box = board->box(x,y);
			if(!trgt_box->isTypeNumber()) trgt_box->setTypeBlank();
		}
	}
}

void generateFixFlag(){

	bool complete_flag = false;
	while(!complete_flag){
	complete_flag = true; // フラグオン
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			//cout << x << "," << y << endl;
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeAllFixed()) continue;
			Direction d = {true,true,true,true};
			
			if(x==0){
				d.w = false;
			}
			else{
				Box* find_box = board->box(x-1,y);
				if(find_box->isTypeAllFixed() && !find_box->isEastLineFixed()){
					d.w = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.w = false;
				}
			}
			if(x==(board->getSizeX()-1)){
				d.e = false;
			}
			else{
				Box* find_box = board->box(x+1,y);
				if(find_box->isTypeAllFixed() && !find_box->isWestLineFixed()){
					d.e = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.e = false;
				}
			}
			if(y==0){
				d.n = false;
			}
			else{
				Box* find_box = board->box(x,y-1);
				if(find_box->isTypeAllFixed() && !find_box->isSouthLineFixed()){
					d.n = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.n = false;
				}
			}
			if(y==(board->getSizeY()-1)){
				d.s = false;
			}
			else{
				Box* find_box = board->box(x,y+1);
				if(find_box->isTypeAllFixed() && !find_box->isNorthLineFixed()){
					d.s = false;
				}
				if(trgt_box->isTypeNumber() && find_box->isTypeNumber()){
					d.s = false;
				}
			}
			
			// 半固定マスに対する操作
			int half_line = -1;
			if(trgt_box->isTypeHalfFixed()){
				// 北にだけ線を持つ
				if(trgt_box->isNorthLineFixed()){
					half_line = NORTH;
					Box* find_box = board->box(x,y-1);
					if(find_box->isEastLineFixed()){
						d.e = false;
					}
					if(find_box->isWestLineFixed()){
						d.w = false;
					}
					
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeHalfFixed() && find_box->isNorthLineFixed()){
							d.w = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.s = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeHalfFixed() && find_box->isNorthLineFixed()){
							d.e = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.w = false; d.s = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}
				// 東だけに線を持つ
				if(trgt_box->isEastLineFixed()){
					half_line = EAST;
					Box* find_box = board->box(x+1,y);
					if(find_box->isNorthLineFixed()){
						d.n = false;
					}
					if(find_box->isSouthLineFixed()){
						d.s = false;
					}
					
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeHalfFixed() && find_box->isEastLineFixed()){
							d.n = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.s = false; d.w = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeHalfFixed() && find_box->isEastLineFixed()){
							d.s = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.w = false;
							}
						}
					}
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}
				// 南だけに線を持つ
				if(trgt_box->isSouthLineFixed()){
					half_line = SOUTH;
					Box* find_box = board->box(x,y+1);
					if(find_box->isEastLineFixed()){
						d.e = false;
					}
					if(find_box->isWestLineFixed()){
						d.w = false;
					}
					
					// 西マス対象
					if(x!=0){
						find_box = board->box(x-1,y);
						if(find_box->isTypeHalfFixed() && find_box->isSouthLineFixed()){
							d.w = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.w = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x-1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.w = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.e = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeHalfFixed() && find_box->isSouthLineFixed()){
							d.e = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.w = false;
							}
						}
					}
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.w = false;
							}
						}
					}
				}
				// 西だけに線を持つ
				if(trgt_box->isWestLineFixed()){
					half_line = WEST;
					Box* find_box = board->box(x-1,y);
					if(find_box->isNorthLineFixed()){
						d.n = false;
					}
					if(find_box->isSouthLineFixed()){
						d.s = false;
					}
					
					// 北マス対象
					if(y!=0){
						find_box = board->box(x,y-1);
						if(find_box->isTypeHalfFixed() && find_box->isWestLineFixed()){
							d.n = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.n = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y-1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.n = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.e = false; d.s = false;
							}
						}
					}
					// 南マス対象
					if(y!=(board->getSizeY()-1)){
						find_box = board->box(x,y+1);
						if(find_box->isTypeHalfFixed() && find_box->isWestLineFixed()){
							d.s = false;
						}
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.s = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x,y+1);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.s = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.e = false;
							}
						}
					}
					// 東マス対象
					if(x!=(board->getSizeX()-1)){
						find_box = board->box(x+1,y);
						if(find_box->isTypeNumber()){
							int trgt_num = getConnectedNumber(x,y);
							if(trgt_num>0 && find_box->getNumber()!=trgt_num){
								d.e = false;
							}
						}
						if(find_box->isTypeHalfFixed()){
							int trgt_num = getConnectedNumber(x,y);
							int find_num = getConnectedNumber(x+1,y);
							if(trgt_num>0 && find_num>0 && trgt_num!=find_num){
								d.e = false;
							}
							if(trgt_num>0 && find_num>0 && trgt_num==find_num){
								d.n = false; d.s = false;
							}
						}
					}
				}
			}
			
			// 数字マスに対する操作
			if(trgt_box->isTypeNumber()){
				// 西マス対象
				if(x!=0){
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x-1,y);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.w = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.e = false; d.s = false;
						}
					}
				}
				// 東マス対象
				if(x!=(board->getSizeX()-1)){
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x+1,y);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.e = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.s = false; d.w = false;
						}
					}
				}
				// 北マス対象
				if(y!=0){
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x,y-1);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.n = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.e = false; d.s = false; d.w = false;
						}
					}
				}
				// 南マス対象
				if(y!=(board->getSizeY()-1)){
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeHalfFixed()){
						int find_num = getConnectedNumber(x,y+1);
						if(find_num>0 && trgt_box->getNumber()!=find_num){
							d.s = false;
						}
						if(find_num>0 && trgt_box->getNumber()==find_num){
							d.n = false; d.e = false; d.w = false;
						}
					}
				}
			}
			
			
			int count = 0;
			if(d.n) count++;
			if(d.e) count++;
			if(d.s) count++;
			if(d.w) count++;
			
			if(trgt_box->isTypeBlank() && count==2){
				complete_flag = false;
				trgt_box->setTypeAllFixed();
				if(d.n && half_line!=NORTH){
					trgt_box->fixNorthLine();
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixSouthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixSouthLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixSouthLine();
						}
					}
				}
				if(d.e && half_line!=EAST){
					trgt_box->fixEastLine();
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixWestLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixWestLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixWestLine();
						}
					}
				}
				if(d.s && half_line!=SOUTH){
					trgt_box->fixSouthLine();
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixNorthLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixNorthLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixNorthLine();
						}
					}
				}
				if(d.w && half_line!=WEST){
					trgt_box->fixWestLine();
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeNumber()){
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else{
							find_box->setTypeAllFixed();
							find_box->fixEastLine();
						}
					}
					else{
						if(find_box->isTypeAllFixed()){
							cout << "Conflict!!" << endl;
							exit(1);
						}
						else if(find_box->isTypeHalfFixed()){
							find_box->setTypeAllFixed();
							find_box->fixEastLine();
						}
						else{
							find_box->setTypeHalfFixed();
							find_box->fixEastLine();
						}
					}
				}
			}
			if(trgt_box->isTypeNumber() && count==1){
				complete_flag = false;
				trgt_box->setTypeAllFixed();
				if(d.n){
					trgt_box->fixNorthLine();
					Box* find_box = board->box(x,y-1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixSouthLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixSouthLine();
					}
				}
				if(d.e){
					trgt_box->fixEastLine();
					Box* find_box = board->box(x+1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixWestLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixWestLine();
					}
				}
				if(d.s){
					trgt_box->fixSouthLine();
					Box* find_box = board->box(x,y+1);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixNorthLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixNorthLine();
					}
				}
				if(d.w){
					trgt_box->fixWestLine();
					Box* find_box = board->box(x-1,y);
					if(find_box->isTypeAllFixed()){
						cout << "Conflict!!" << endl;
						exit(1);
					}
					else if(find_box->isTypeHalfFixed()){
						find_box->setTypeAllFixed();
						find_box->fixEastLine();
					}
					else{
						find_box->setTypeHalfFixed();
						find_box->fixEastLine();
					}
				}
			}
		}
	}
	
	}
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeAllFixed()) cout << "2";
			else if(trgt_box->isTypeHalfFixed()) cout << "1";
			else cout << "0";
		}
		cout << endl;
	}
	
	// for(int y=0;y<board->getSizeY();y++){
		// for(int x=0;x<board->getSizeX();x++){
			// Box* trgt_box = board->box(x,y);
			// if(trgt_box->isNorthLineFixed()){
				// cout << " N";
			// }
			// else{
				// cout << " +";
			// }
			// if(trgt_box->isEastLineFixed()){
				// cout << " E";
			// }
			// else{
				// cout << " +";
			// }
			// cout << " ";
		// }
		// cout << endl;
		// for(int x=0;x<board->getSizeX();x++){
			// Box* trgt_box = board->box(x,y);
			// if(trgt_box->isWestLineFixed()){
				// cout << " W";
			// }
			// else{
				// cout << " +";
			// }
			// if(trgt_box->isSouthLineFixed()){
				// cout << " S";
			// }
			// else{
				// cout << " +";
			// }
			// cout << " ";
		// }
		// cout << endl;
	// }
}

int getConnectedNumber(int x,int y){

	Box* trgt_box = board->box(x,y);
	
	queue<Search> qu;
	if(trgt_box->isNorthLineFixed()){
		Search trgt = {x,y-1,SOUTH,-1};
		qu.push(trgt);
	}
	if(trgt_box->isEastLineFixed()){
		Search trgt = {x+1,y,WEST,-1};
		qu.push(trgt);
	}
	if(trgt_box->isSouthLineFixed()){
		Search trgt = {x,y+1,NORTH,-1};
		qu.push(trgt);
	}
	if(trgt_box->isWestLineFixed()){
		Search trgt = {x-1,y,EAST,-1};
		qu.push(trgt);
	}
	
	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		trgt_box = board->box(trgt.x,trgt.y);
		
		if(trgt_box->isTypeNumber()){
			return trgt_box->getNumber();
		}
		
		if(trgt_box->isTypeHalfFixed()){
			continue;
		}
		
		// 北方向
		if(trgt_box->isNorthLineFixed() && trgt.d!=NORTH){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
		}
		// 東方向
		if(trgt_box->isEastLineFixed() && trgt.d!=EAST){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
		}
		// 南方向
		if(trgt_box->isSouthLineFixed() && trgt.d!=SOUTH){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
		}
		// 西方向
		if(trgt_box->isWestLineFixed() && trgt.d!=WEST){
			Search next = {trgt.x-1,trgt.y,EAST,trgt.d};
			qu.push(next);
		}
	}
	
	return -1;
}

void setAdjacentNumberProc(int x,int y){

	adj_num.clear();
	Box* trgt_box = board->box(x,y);
	
	queue<Search> qu;
	if(trgt_box->isNorthLineFixed()){
		Search trgt = {x,y-1,SOUTH,-1};
		qu.push(trgt);
	}
	if(trgt_box->isEastLineFixed()){
		Search trgt = {x+1,y,WEST,-1};
		qu.push(trgt);
	}
	if(trgt_box->isSouthLineFixed()){
		Search trgt = {x,y+1,NORTH,-1};
		qu.push(trgt);
	}
	if(trgt_box->isWestLineFixed()){
		Search trgt = {x-1,y,EAST,-1};
		qu.push(trgt);
	}
	
	while(!qu.empty()){
		
		Search trgt = qu.front();
		qu.pop();
		
		trgt_box = board->box(trgt.x,trgt.y);
				
		if(trgt_box->isTypeNumber() || trgt_box->isTypeHalfFixed()){
			continue;
		}
		
		// 北方向
		if(trgt_box->isNorthLineFixed() && trgt.d!=NORTH){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
			// NORTH & trgt.d に線が存在
			// ->それ以外の方向に数字があったら，それは隣接する数字
			setAdjacentNumber(trgt.x,trgt.y,NORTH,trgt.d);
		}
		// 東方向
		if(trgt_box->isEastLineFixed() && trgt.d!=EAST){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
			// EAST & trgt.d に線が存在
			// ->それ以外の方向に数字があったら，それは隣接する数字
			setAdjacentNumber(trgt.x,trgt.y,EAST,trgt.d);
		}
		// 南方向
		if(trgt_box->isSouthLineFixed() && trgt.d!=SOUTH){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
			// SOUTH & trgt.d に線が存在
			// ->それ以外の方向に数字があったら，それは隣接する数字
			setAdjacentNumber(trgt.x,trgt.y,SOUTH,trgt.d);
		}
		// 西方向
		if(trgt_box->isWestLineFixed() && trgt.d!=WEST){
			Search next = {trgt.x-1,trgt.y,EAST,trgt.d};
			qu.push(next);
			// WEST & trgt.d に線が存在
			// ->それ以外の方向に数字があったら，それは隣接する数字
			setAdjacentNumber(trgt.x,trgt.y,WEST,trgt.d);
		}
	}
}

void setAdjacentNumber(int x,int y,int d_1,int d_2){

	// 北を調査するか？
	if(y!=0 && d_1!=NORTH && d_2!=NORTH){
		Box* trgt_box = board->box(x,y-1);
		if(trgt_box->isTypeNumber()){
			adj_num.push_back(trgt_box->getNumber());
		}
	}
	// 東を調査するか？
	if(x!=(board->getSizeX()-1) && d_1!=EAST && d_2!=EAST){
		Box* trgt_box = board->box(x+1,y);
		if(trgt_box->isTypeNumber()){
			adj_num.push_back(trgt_box->getNumber());
		}
	}
	// 南を調査するか？
	if(y!=(board->getSizeY()-1) && d_1!=SOUTH && d_2!=SOUTH){
		Box* trgt_box = board->box(x,y+1);
		if(trgt_box->isTypeNumber()){
			adj_num.push_back(trgt_box->getNumber());
		}
	}
	// 西を調査するか？
	if(x!=0 && d_1!=WEST && d_2!=WEST){
		Box* trgt_box = board->box(x-1,y);
		if(trgt_box->isTypeNumber()){
			adj_num.push_back(trgt_box->getNumber());
		}
	}
}

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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
	int min_cost = INT_MAX;
	vector<int> min_direction_array;
	for(int a=0;a<8;a++){
		if(adj_cost[a] > 10000) continue;
		if(adj_cost[a] < min_cost){
			min_direction_array.clear();
			min_direction_array.push_back(a);
			min_cost = adj_cost[a];
		}
		else if(adj_cost[a] == min_cost){
			min_direction_array.push_back(a);
		}
	}
	if((int)(min_direction_array.size())==0){
		return false;
	}
	int adj_count = rand() % (int)(min_direction_array.size());
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
		trgt_line->pushIntToTrack(now_x+now_y*board->getSizeX());
		
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
		}
		
		vector<int> next_direction_array;
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		//cout << (int)(next_direction_array.size()) << endl;
		
		int next_count = rand() % (int)(next_direction_array.size());
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
	vector<int>* trgt_track = trgt_line->getTrack();
	
	while(retry){
		retry = false;
		vector<int> tmp_track;
		for(int i=0;i<(int)(trgt_track->size());i++){
			tmp_track.push_back((*trgt_track)[i]);
		}
		trgt_track->clear();
		for(int i=0;i<(int)(tmp_track.size());i++){
			if(i>=(int)(tmp_track.size())-2){
				trgt_track->push_back(tmp_track[i]);
				continue;
			}
			if(tmp_track[i+2]==tmp_track[i]){
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
		if(c==NORTH && trgt_box->isNorthLineFixed()
		|| c==EAST && trgt_box->isEastLineFixed()
		|| c==SOUTH && trgt_box->isSouthLineFixed()
		|| c==WEST && trgt_box->isWestLineFixed()){
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
	vector<int>* trgt_track = trgt_line->getTrack();
	
	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int new_x = (*trgt_track)[0] % board->getSizeX();
	int new_y = (*trgt_track)[0] / board->getSizeX();
	
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
		new_x = (*trgt_track)[i] % board->getSizeX();
		new_y = (*trgt_track)[i] / board->getSizeX();
		
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
	vector<int>* trgt_track = trgt_line->getTrack();
	
	int old_x = trgt_line->getSinkX();
	int old_y = trgt_line->getSinkY();
	int new_x = (*trgt_track)[0] % board->getSizeX();
	int new_y = (*trgt_track)[0] / board->getSizeX();
	
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
		new_x = (*trgt_track)[i] % board->getSizeX();
		new_y = (*trgt_track)[i] / board->getSizeX();
		
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

bool isFinished(){

	map<int,map<int,int> > for_check;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			for_check[y][x] = 0;
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		vector<int>* trgt_track = trgt_line->getTrack();
		for(int j=0;j<(int)(trgt_track->size());j++){
			int point = (*trgt_track)[j];
			int point_x = point % board->getSizeX();
			int point_y = point / board->getSizeX();
			if(for_check[point_y][point_x] > 0){ return false; }
			else{ for_check[point_y][point_x] = 1; }
		}
	}
	
	return true;
}

// ライン不通過回数をリセット（全マスについて）
// 各ラインの中間ポート利用候補をリセット
void resetCandidate(){

	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			trgt_box->resetSelfCount();
			trgt_box->setNonCandidate();
		}
	}
	
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		trgt_line->setNonCandidate();
	}
}	

// ライン不通過回数をカウント（数字マス以外）
// 回数が一定割合以上なら，コメント出力<-
void checkLineNonPassed(){

	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeNumber() || trgt_box->isTypeAllFixed()) continue;
			if(countLineNum(x,y)<1){
				trgt_box->incrementSelfCount();
			}
			
			if(trgt_box->getSelfCount()>LIMIT){
				//cout << "(" << x << "," << y << ")" << endl;
				trgt_box->setCandidate();
				trgt_box->resetSelfCount();
			}
		}
	}
}

// 中間ポートを利用する場合のルーティング（ソースから中間まで）
bool routingSourceToI(int trgt_line_id){

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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
	int now_x = trgt_line->getInterX();
	int now_y = trgt_line->getInterY();
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
	int min_cost = INT_MAX;
	vector<int> min_direction_array;
	for(int a=0;a<8;a++){
		if(adj_cost[a] > 10000) continue;
		if(adj_cost[a] < min_cost){
			min_direction_array.clear();
			min_direction_array.push_back(a);
			min_cost = adj_cost[a];
		}
		else if(adj_cost[a] == min_cost){
			min_direction_array.push_back(a);
		}
	}
	if((int)(min_direction_array.size())==0){
		return false;
	}
	int adj_count = rand() % (int)(min_direction_array.size());
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
		trgt_line->pushIntToTrack(now_x+now_y*board->getSizeX());
		
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
		}
		
		vector<int> next_direction_array;
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		//cout << (int)(next_direction_array.size()) << endl;
		
		int next_count = rand() % (int)(next_direction_array.size());
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
	vector<int>* trgt_track = trgt_line->getTrack();
	
	while(retry){
		retry = false;
		vector<int> tmp_track;
		for(int i=0;i<(int)(trgt_track->size());i++){
			tmp_track.push_back((*trgt_track)[i]);
		}
		trgt_track->clear();
		for(int i=0;i<(int)(tmp_track.size());i++){
			if(i>=(int)(tmp_track.size())-2){
				trgt_track->push_back(tmp_track[i]);
				continue;
			}
			if(tmp_track[i+2]==tmp_track[i]){
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

// 中間ポートを利用する場合のルーティング（中間からシンクまで）
bool routingIToSink(int trgt_line_id){

	Line* trgt_line = board->line(trgt_line_id);
	
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
	
	// 通れないマスを規定
	vector<int>* trgt_track = trgt_line->getTrack();
	map<int,map<int,bool> > can_pass;
	for(int y=-1;y<=board->getSizeY();y++){
		for(int x=-1;x<=board->getSizeX();x++){
			can_pass[y][x] = true;
		}
	}
	for(int i=0;i<(int)(trgt_track->size());i++){
		int tmp_x = (*trgt_track)[i] % board->getSizeX();
		int tmp_y = (*trgt_track)[i] / board->getSizeX();
		can_pass[tmp_y][tmp_x] = false;
		can_pass[tmp_y][tmp_x-1] = false;
		can_pass[tmp_y][tmp_x+1] = false;
		can_pass[tmp_y-1][tmp_x] = false;
		can_pass[tmp_y+1][tmp_x] = false;
	}
	
	// ソースから中間までの経路を一時保存
	vector<int> before_track;
	for(int i=0;i<(int)(trgt_track->size());i++){
		before_track.push_back((*trgt_track)[i]);
	}
	trgt_track->clear();
	
	
	int start_x = trgt_line->getInterX();
	int start_y = trgt_line->getInterY();
	
	IntraBox* start = my_board[start_y][start_x];
	start->ne = 0; start->nw = 0;
	start->se = 0; start->sw = 0;
	
	queue<Search> qu;
	// 北方向
	if(can_pass[start_y-1][start_x] && isInserted(start_x,start_y-1,SOUTH) && isFixed(start_x,start_y,NORTH,-1,trgt_line_id)){
		Search trgt = {start_x,start_y-1,SOUTH,-1};
		qu.push(trgt);
	}
	// 東方向
	if(can_pass[start_y][start_x+1] && isInserted(start_x+1,start_y,WEST) && isFixed(start_x,start_y,EAST,-1,trgt_line_id)){
		Search trgt = {start_x+1,start_y,WEST,-1};
		qu.push(trgt);
	}
	// 南方向
	if(can_pass[start_y+1][start_x] && isInserted(start_x,start_y+1,NORTH) && isFixed(start_x,start_y,SOUTH,-1,trgt_line_id)){
		Search trgt = {start_x,start_y+1,NORTH,-1};
		qu.push(trgt);
	}
	// 西方向
	if(can_pass[start_y][start_x-1] && isInserted(start_x-1,start_y,EAST) && isFixed(start_x,start_y,WEST,-1,trgt_line_id)){
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
			if(touch_count < 0){ cout << "error!" << endl; exit(1); }
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
		if(trgt.d!=NORTH && can_pass[trgt.y-1][trgt.x] && isInserted(trgt.x,trgt.y-1,SOUTH) && isFixed(trgt.x,trgt.y,NORTH,trgt.d,trgt_line_id)){
			Search next = {trgt.x,trgt.y-1,SOUTH,trgt.d};
			qu.push(next);
		}
		// 東方向
		if(trgt.d!=EAST && can_pass[trgt.y][trgt.x+1] && isInserted(trgt.x+1,trgt.y,WEST) && isFixed(trgt.x,trgt.y,EAST,trgt.d,trgt_line_id)){
			Search next = {trgt.x+1,trgt.y,WEST,trgt.d};
			qu.push(next);
		}
		// 南方向
		if(trgt.d!=SOUTH && can_pass[trgt.y+1][trgt.x] && isInserted(trgt.x,trgt.y+1,NORTH) && isFixed(trgt.x,trgt.y,SOUTH,trgt.d,trgt_line_id)){
			Search next = {trgt.x,trgt.y+1,NORTH,trgt.d};
			qu.push(next);
		}
		// 西方向
		if(trgt.d!=WEST && can_pass[trgt.y][trgt.x-1] && isInserted(trgt.x-1,trgt.y,EAST) && isFixed(trgt.x,trgt.y,WEST,trgt.d,trgt_line_id)){
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
	int min_cost = INT_MAX;
	vector<int> min_direction_array;
	for(int a=0;a<8;a++){
		if(adj_cost[a] > 10000) continue;
		if(adj_cost[a] < min_cost){
			min_direction_array.clear();
			min_direction_array.push_back(a);
			min_cost = adj_cost[a];
		}
		else if(adj_cost[a] == min_cost){
			min_direction_array.push_back(a);
		}
	}
	if((int)(min_direction_array.size())==0){
		for(int i=0;i<(int)(before_track.size());i++){
			trgt_track->push_back(before_track[i]);
		}
		return false;
	}
	int adj_count = rand() % (int)(min_direction_array.size());
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
		trgt_line->pushIntToTrack(now_x+now_y*board->getSizeX());
		
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
		}
		
		vector<int> next_direction_array;
		if(trgt_d.n) next_direction_array.push_back(NORTH);
		if(trgt_d.e) next_direction_array.push_back(EAST);
		if(trgt_d.s) next_direction_array.push_back(SOUTH);
		if(trgt_d.w) next_direction_array.push_back(WEST);
		//cout << (int)(next_direction_array.size()) << endl;
		
		int next_count = rand() % (int)(next_direction_array.size());
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
	while(retry){
		retry = false;
		vector<int> tmp_track;
		for(int i=0;i<(int)(trgt_track->size());i++){
			tmp_track.push_back((*trgt_track)[i]);
		}
		trgt_track->clear();
		for(int i=0;i<(int)(tmp_track.size());i++){
			if(i>=(int)(tmp_track.size())-2){
				trgt_track->push_back(tmp_track[i]);
				continue;
			}
			if(tmp_track[i+2]==tmp_track[i]){
				retry = true;
				i++;
				continue;
			}
			trgt_track->push_back(tmp_track[i]);
		}
	}
	
	// 中間ポートと中間からソースまでの経路を追加
	trgt_track->push_back(trgt_line->getInterX()+trgt_line->getInterY()*board->getSizeX());
	for(int i=0;i<(int)(before_track.size());i++){
		trgt_track->push_back(before_track[i]);
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

void checkCandidateLine(int x,int y){
	
	int now_x, now_y;
	
	// 北方向へ
	now_x = x; now_y = y;
	while(now_y>0){
		now_y = now_y - 1;
		Box* trgt_box = board->box(now_x,now_y);
		if(trgt_box->isTypeNumber()){
			if(!trgt_box->isTypeAllFixed() || trgt_box->isSouthLineFixed()){
				Line* trgt_line = board->line(trgt_box->getNumber());
				trgt_line->setCandidate();
			}
			break;
		}
		if(countLineNum(now_x,now_y)>0){
			for(int i=1;i<=board->getLineNum();i++){
				Line* trgt_line = board->line(i);
				vector<int>* trgt_track = trgt_line->getTrack();
				for(int j=0;j<(int)(trgt_track->size());j++){
					int point = (*trgt_track)[j];
					int point_x = point % board->getSizeX();
					int point_y = point / board->getSizeX();
					if(point_x==now_x && point_y==now_y){
						trgt_line->setCandidate();
						break;
					}
				}
			}
			break;
		}
	}
	
	// 東方向へ
	now_x = x; now_y = y;
	while(now_x<(board->getSizeX()-1)){
		now_x = now_x + 1;
		Box* trgt_box = board->box(now_x,now_y);
		if(trgt_box->isTypeNumber()){
			if(!trgt_box->isTypeAllFixed() || trgt_box->isWestLineFixed()){
				Line* trgt_line = board->line(trgt_box->getNumber());
				trgt_line->setCandidate();
			}
			break;
		}
		if(countLineNum(now_x,now_y)>0){
			for(int i=1;i<=board->getLineNum();i++){
				Line* trgt_line = board->line(i);
				vector<int>* trgt_track = trgt_line->getTrack();
				for(int j=0;j<(int)(trgt_track->size());j++){
					int point = (*trgt_track)[j];
					int point_x = point % board->getSizeX();
					int point_y = point / board->getSizeX();
					if(point_x==now_x && point_y==now_y){
						trgt_line->setCandidate();
						break;
					}
				}
			}
			break;
		}
	}
	
	// 南方向へ
	now_x = x; now_y = y;
	while(now_y<(board->getSizeY()-1)){
		now_y = now_y + 1;
		Box* trgt_box = board->box(now_x,now_y);
		if(trgt_box->isTypeNumber()){
			if(!trgt_box->isTypeAllFixed() || trgt_box->isNorthLineFixed()){
				Line* trgt_line = board->line(trgt_box->getNumber());
				trgt_line->setCandidate();
			}
			break;
		}
		if(countLineNum(now_x,now_y)>0){
			for(int i=1;i<=board->getLineNum();i++){
				Line* trgt_line = board->line(i);
				vector<int>* trgt_track = trgt_line->getTrack();
				for(int j=0;j<(int)(trgt_track->size());j++){
					int point = (*trgt_track)[j];
					int point_x = point % board->getSizeX();
					int point_y = point / board->getSizeX();
					if(point_x==now_x && point_y==now_y){
						trgt_line->setCandidate();
						break;
					}
				}
			}
			break;
		}
	}
	
	// 西方向へ
	now_x = x; now_y = y;
	while(now_x>0){
		now_x = now_x - 1;
		Box* trgt_box = board->box(now_x,now_y);
		if(trgt_box->isTypeNumber()){
			if(!trgt_box->isTypeAllFixed() || trgt_box->isEastLineFixed()){
				Line* trgt_line = board->line(trgt_box->getNumber());
				trgt_line->setCandidate();
			}
			break;
		}
		if(countLineNum(now_x,now_y)>0){
			for(int i=1;i<=board->getLineNum();i++){
				Line* trgt_line = board->line(i);
				vector<int>* trgt_track = trgt_line->getTrack();
				for(int j=0;j<(int)(trgt_track->size());j++){
					int point = (*trgt_track)[j];
					int point_x = point % board->getSizeX();
					int point_y = point / board->getSizeX();
					if(point_x==now_x && point_y==now_y){
						trgt_line->setCandidate();
						break;
					}
				}
			}
			break;
		}
	}
}
