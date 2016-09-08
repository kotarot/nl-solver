#include "./main.hpp"
#include "./route.hpp"
#include "./utils.hpp"

/*******************************************************/
/** グローバル変数定義 **/
/*******************************************************/

Board* board; // 対象ボード
int line_to_viaid[100];			// Lineと対応するViaの内部ID対応
int line_to_via_priority[100];	// Lineに対応するViaの優先度(*数字が高いほど重要！)
int via_priority;				// 採用するViaの優先度

int penalty_T; // penalty of "touch"
int penalty_C; // penalty of "cross"
int penalty_V; // penalty of "via duplication"

void usage() {
	cerr << "Usage: solver [--loop <loop-number>] [--output <output-file>] [--debug] [--print] input-file" << endl;
	exit(-1);
}

void version() {
	cerr << "Version: nl-solver 2016.0.0" << endl;
	exit(-1);
}

int main(int argc, char *argv[]){
	// Options
	char *in_filename  = NULL; // 問題ファイル名
	char *out_filename = NULL; // 出力解答ファイル名
	int outer_loops = O_LOOP;  // 外ループ回数
	bool debug_option = false; // デバッグ出力 (ルーティング)
	bool print_option = false; // デバッグ出力 (問題＆解答)

	// Options 取得
	struct option longopts[] = {
		{"loop",     required_argument, NULL, 'l'},
		{"output",   required_argument, NULL, 'o'},
		{"debug",    no_argument,       NULL, 'd'},
		{"print",    no_argument,       NULL, 'p'},
		{"version",  no_argument,       NULL, 'v'},
		{"help",     no_argument,       NULL, 'h'},
		{0, 0, 0, 0}
	};
	int opt, optidx;
	while ((opt = getopt_long(argc, argv, "l:o:dpvh", longopts, &optidx)) != -1) {
		switch (opt) {
			case 'l':
				outer_loops = atoi(optarg);
				break;
			case 'o':
				out_filename = optarg;
				break;
			case 'd':
				debug_option = true;
				break;
			case 'p':
				print_option = true;
				break;
			case 'v':
				version();
			case 'h':
			case ':':
			case '?':
			default:
				usage();
		}
	}
	if (argc <= optind) {
		usage();
	}
	in_filename = argv[optind];
	assert(in_filename != NULL);

	clock_t start_time, finish_time;
	start_time = clock();

	initialize(in_filename); // 問題盤の生成

if( print_option ) { printBoard(); }

	// 乱数の初期化
	mt_init_genrand((unsigned long)time(NULL));
	// ペナルティの初期化
	penalty_T = 0;
	penalty_C = 0;
	penalty_V = 0;
	
	// 初期ルーティング
	for(int i=1;i<=board->getLineNum();i++){
		// 数字が隣接する場合スキップ
		if(board->line(i)->getHasLine() == false) continue;
		
		if( !routing(i, debug_option) ){
			cerr << "Cannot solve!! (error: 1)" << endl;
			exit(1);
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		// 数字が隣接する場合スキップ
		if(board->line(i)->getHasLine() == false) continue;
		
		recordLine(i);
	}

	via_priority = 0;
	
	// 探索スタート!!
	// 外ループ
	for (int m = 2; m <= outer_loops + 1; m++) {
	
		// 解導出フラグ
		bool complete = false;

		if (m / 10 > via_priority){
			via_priority++;
if( print_option ) cout << "Via priority is " << via_priority << endl;
		}
		
		// 内ループ
		for (int n = 1; n <= I_LOOP; n++) { // 内ループ

			//cout << m-1 << ":" << n << endl;

			// 問題中で数字が隣接していないラインを選択
			int id;
			do {
				id = (int)mt_genrand_int32(1, board->getLineNum());
			} while (board->line(id)->getHasLine() == false);

			// 経路の削除
			deleteLine(id);
			
			// ペナルティの設定
			penalty_T = (int)(NT * (mt_genrand_int32(0, m - 1)));
			penalty_C = (int)(NC * (mt_genrand_int32(0, m - 1)));
			penalty_V = (int)(NV * (mt_genrand_int32(0, m - 1)));

			// ビア指定
			// int via_idx = 1; // ビア番号
			// board->line(id)->setSpecifiedVia(via_idx);
			// ビア指定解除
			// board->line(id)->setSpecifiedVia(NOT_USE);

			// Via指定
			if(line_to_viaid[id] != 0){
				if(line_to_via_priority[id] >= via_priority)
					board->line(id)->setSpecifiedVia(line_to_viaid[id]);
				else
					board->line(id)->setSpecifiedVia(NOT_USE);
			}

			// 経路の探索
			if ( !routing(id, debug_option) ) {
				cerr << "Cannot solve!! (error: 2)" << endl; // 失敗したらプログラム終了
				exit(2);
			}
			// 経路の記録
			recordLine(id);

			// 終了判定（解導出できた場合，正解を出力）
			if(isFinished()){

				// 最終ルーティング
				for(int i=1;i<=board->getLineNum();i++){
					// 数字が隣接する場合スキップ
					if(board->line(i)->getHasLine() == false) continue;
		
					// 経路の削除
					deleteLine(i);

					if( !final_routing(i, debug_option) ){
						cerr << "Cannot solve!! (error: 3)" << endl;
						exit(3);
					}

					// 経路の記録
					recordLine(i);
				}
	
				finish_time = clock();

if( print_option ) { printSolution(); }

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

				complete = true;
				break;
			}
		}
		if(complete) break; // 正解出力後は外ループも脱出
	}
	
	
	// 解導出できなかった場合
	if(!isFinished()){
		finish_time = clock();
		
// 現状の結果を出力
if( print_option ) {
	for(int i=1;i<=board->getLineNum();i++){
		printLine(i);
	}
	cout << endl;
}

		cout << "SUMMARY" << endl;
		cout << "-------" << endl;
		cout << " - filename:   " << in_filename << endl;
		cout << " - size:       " << board->getSizeX() << " x " << board->getSizeY() << " x " << board->getSizeZ() << endl;
		cout << " - iterations: " << outer_loops << endl;
		cout << " - CPU time:   "
		     << ((double)(finish_time - start_time) / (double)CLOCKS_PER_SEC)
		     << " sec" << endl;

		cerr << "Cannot solve!! (error: 4)" << endl;
		exit(4);
	}

	delete board;
	return 0;
}

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
				line_to_viaid[i] = vid;
				line_to_via_priority[i] = vp;
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

bool isFinished(){

	int for_check[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				for_check[z][y][x] = 0;
			}
		}
	}
	for(int i=1;i<=board->getLineNum();i++){
		Line* trgt_line = board->line(i);
		vector<Point>* trgt_track = trgt_line->getTrack();
		for(int j=0;j<(int)(trgt_track->size());j++){
			Point p = (*trgt_track)[j];
			int point_x = p.x;
			int point_y = p.y;
			int point_z = p.z;
			if(for_check[point_z][point_y][point_x] > 0){ return false; }
			else{ for_check[point_z][point_y][point_x] = 1; }
		}
	}
	
	return true;
}

