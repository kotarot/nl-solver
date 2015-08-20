#include "main.hpp"
#include "utils.hpp"

extern Board* board;

// 2桁の整数をアルファベットに変換
char changeIntToChar(int n){
	if (0 <= n && n < 10) {
		return (char)(n + '0');
	} else if (n <= 35) {
		return (char)(n - 10 + 'A');
	}
	return '#';
}

// 問題盤を表示
void printBoard(){

	cout << endl;
	cout << "PROBLEM" << endl;
	cout << "=======" << endl;

	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				cout << " . ";
			}else{
				int trgt_num = trgt_box->getNumber();
				cout << " " << changeIntToChar(trgt_num) << " ";
			}
		}
		cout << endl;
	}
	cout << endl;
}

// 現状のラインを表示
void printLine(int i){

	cout << endl;
	cout << "Line " << i << endl;
	Line* trgt_line = board->line(i);
	vector<Point>* trgt_track = trgt_line->getTrack();
	
	map<int,map<int,int> > for_print;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				for_print[y][x] = -1;
			}
			else{
				for_print[y][x] = trgt_box->getNumber();
			}
		}
	}
	for(int j=0;j<(int)(trgt_track->size());j++){
		Point p = (*trgt_track)[j];
		int point_x = p.x;
		int point_y = p.y;
		for_print[point_y][point_x] = -2;
	}
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] == -2) cout << " @";
			else if(for_print[y][x] == -1) cout << " .";
			else{
				cout << " " << changeIntToChar(for_print[y][x]);
			}
		}
		cout << endl;
	}
}

// ラインナンバーから色をマッピング
// 41m ~ 47m まで 7色
string getcolorescape(int n) {
	int color = ((n - 1) % 7) + 41;
	stringstream ss;
	ss << color;
	string p = "\033[";
	string s = "m";
	return p + ss.str() + s;
}

// 正解を表示（正解表示は数字２桁）
void printSolution(){
	map<int,map<int,int> > for_print;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				for_print[y][x] = -1;
			}
			else{
				for_print[y][x] = trgt_box->getNumber();
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
			for_print[point_y][point_x] = i;
		}
	}
	
	cout << endl;
	cout << "SOLUTION" << endl;
	cout << "========" << endl;

	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] < 0){
				// 線が引かれていないマス："00"表示
				cout << " \033[37m00\033[0m";
			}else{
				// その他(2桁表示)
				int n = for_print[y][x];
				cout << " " << getcolorescape(n) << setfill('0') << setw(2) << n << "\033[0m";
			}
			//if(x!=(board->getSizeX()-1)){
			//	cout << ",";
			//}
		}
		cout << endl;
	}
	cout << endl;
}

// 正解を表示 (ファイル出力)
void printSolutionToFile(char *filename) {
	map<int,map<int,int> > for_print;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				for_print[y][x] = -1;
			}
			else{
				for_print[y][x] = trgt_box->getNumber();
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
			for_print[point_y][point_x] = i;
		}
	}

	ofstream ofs(filename);
	ofs << "SIZE " << board->getSizeX() << "X" << board->getSizeY() << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] < 0){
				// 線が引かれていないマス："00"表示
				ofs << "00";
			}
			else{
				// その他(2桁表示)
				ofs << setfill('0') << setw(2) << for_print[y][x];
			}
			if(x!=(board->getSizeX()-1)){
				ofs << ",";
			}
		}
		ofs << endl;
	}
}

// ================================ //
// メルセンヌ・ツイスタ
// ================================ //
#include "mt19937ar.c"

void mt_init_genrand(unsigned long s) {
	init_genrand(s);
}

// AからBの範囲の整数の乱数が欲しいとき
// 参考 http://www.sat.t.u-tokyo.ac.jp/~omi/random_variables_generation.html
unsigned long mt_genrand_int32(int a, int b) {
	return genrand_int32() % (b - a + 1) + a;
}
