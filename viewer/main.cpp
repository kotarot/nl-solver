#include "./main.hpp"

Board *board; // 対象ボード


void usage() {
	cerr << "Usage: viewer [--answer] input-file" << endl;
	exit(-1);
}

void version() {
	cerr << "Version: nl-viewer 2015" << endl;
	exit(-1);
}

int main(int argc, char *argv[]) {
	// Options
	char *filename  = NULL; // 問題/解答ファイル名
	bool answer = false;    // 入力ファイルモード (問題 or 解答)

	// Options 取得
	struct option longopts[] = {
		{"answer",  no_argument, NULL, 'a'},
		{"version", no_argument, NULL, 'v'},
		{"help",    no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};
	int opt, optidx;
	while ((opt = getopt_long(argc, argv, "avh", longopts, &optidx)) != -1) {
		switch (opt) {
			case 'a':
				answer = true;
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
	filename = argv[optind];
	assert(filename != NULL);

	// 問題ファイル入力モード
	if (!answer) {
		initialize(filename); // 問題盤の生成
		printBoard(); // 問題盤の表示
	}
	// 解答ファイル入力モード
	else {
		assert(!"Not implemented.");
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
	
	int size_x, size_y;
	int line_num;
	map<int,int> x_0, y_0, x_1, y_1;
	map<int,bool> adjacents; // 初期状態で数字が隣接している

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
			
			// 初期状態で数字が隣接しているか判断
			int dx = x_0[i] - x_1[i];
			int dy = y_0[i] - y_1[i];
			if ((dx == 0 && (dy == 1 || dy == -1)) || ((dx == 1 || dx == -1) && dy == 0)) {
				adjacents[i] = true;
			} else {
				adjacents[i] = false;
			}
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
		trgt_line->setHasLine(!adjacents[i]);
	}
	
	for(int y=0;y<size_y;y++){
		for(int x=0;x<size_x;x++){
			Box* trgt_box = board->box(x,y);
			if(!trgt_box->isTypeNumber()) trgt_box->setTypeBlank();
		}
	}
}
