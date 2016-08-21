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

	for(int z=0;z<board->getSizeZ();z++){
		cout << "LAYER " << z+1 << endl;
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				Box* trgt_box = board->box(x,y,z);
				if(trgt_box->isTypeBlank()){
					cout << "[.]";
				}
				else if(trgt_box->isTypeNumber()){
					int trgt_num = trgt_box->getIndex();
					cout << "[" << changeIntToChar(trgt_num) << "]";
				}
				else if(trgt_box->isTypeVia()){
					int trgt_via = trgt_box->getIndex();
					cout << "(" << changeIntToChar(trgt_via) << ")";
				}
				else{
					cout << "(-)";
				}
			}
			cout << endl;
		}
		cout << endl;
	}
}

// ラインナンバーから色をマッピング (Foreground colors)
// 31m ~ 37m まで 7色
string getcolorescape_fore(int n) {
	int color = ((n - 1) % 7) + 31;
	stringstream ss;
	ss << color;
	string p = "\033[";
	string s = "m";
	return p + ss.str() + s;
}
// ラインナンバーから色をマッピング (Background colors)
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
	map<int,map<int,map<int,int> > > for_print;
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				Box* trgt_box = board->box(x,y,z);
				if(trgt_box->isTypeBlank()){
					for_print[z][y][x] = -1;
				}else{
					for_print[z][y][x] = trgt_box->getIndex();
				}
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
			for_print[point_z][point_y][point_x] = i;
		}
	}
	for(int i=1;i<=board->getViaNum();i++){
		Via* trgt_via = board->via(i);
		int via_x = trgt_via->getSourceX();
		int via_y = trgt_via->getSourceY();
		int via_z = trgt_via->getSourceZ();
		int line_num = for_print[via_z][via_y][via_x]; 
		for(int z=via_z+1;z<trgt_via->getSinkZ();z++){
			for_print[z][via_y][via_x] = line_num;
		}
	}
	
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
}

// 正解を表示 (ファイル出力)
void printSolutionToFile(char *filename) {
	map<int,map<int,map<int,int> > > for_print;
	for(int z=0;z<board->getSizeZ();z++){
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				Box* trgt_box = board->box(x,y,z);
				if(trgt_box->isTypeBlank()){
					for_print[z][y][x] = -1;
				}else{
					for_print[z][y][x] = trgt_box->getIndex();
				}
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
			for_print[point_z][point_y][point_x] = i;
		}
	}
	for(int i=1;i<=board->getViaNum();i++){
		Via* trgt_via = board->via(i);
		int via_x = trgt_via->getSourceX();
		int via_y = trgt_via->getSourceY();
		int via_z = trgt_via->getSourceZ();
		int line_num = for_print[via_z][via_y][via_x]; 
		for(int z=via_z+1;z<trgt_via->getSinkZ();z++){
			for_print[z][via_y][via_x] = line_num;
		}
	}

	ofstream ofs(filename);
	ofs << "SIZE " << board->getSizeX() << "X" << board->getSizeY() << "X" << board->getSizeZ() << endl;
	for(int z=0;z<board->getSizeZ();z++){
		ofs << "LAYER " << z+1 << endl;
		for(int y=0;y<board->getSizeY();y++){
			for(int x=0;x<board->getSizeX();x++){
				if(for_print[z][y][x] < 0){
					// 線が引かれていないマス："00"表示
					ofs << "00";
				}
				else{
					// その他(2桁表示)
					ofs << setfill('0') << setw(2) << for_print[z][y][x];
				}
				if(x!=(board->getSizeX()-1)){
					ofs << ",";
				}
			}
			ofs << endl;
		}
		if(z!=(board->getSizeZ()-1)){
			ofs << endl;
		}
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

