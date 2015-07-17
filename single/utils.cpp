#include "main.hpp"
#include "utils.hpp"

extern Board* board;

// ２桁の整数をアルファベットに変換（３５以下）
char changeIntToChar(int n){
	
	switch(n){
		case 10: return 'A'; break;
		case 11: return 'B'; break;
		case 12: return 'C'; break;
		case 13: return 'D'; break;
		case 14: return 'E'; break;
		case 15: return 'F'; break;
		case 16: return 'G'; break;
		case 17: return 'H'; break;
		case 18: return 'I'; break;
		case 19: return 'J'; break;
		case 20: return 'K'; break;
		case 21: return 'L'; break;
		case 22: return 'M'; break;
		case 23: return 'N'; break;
		case 24: return 'O'; break;
		case 25: return 'P'; break;
		case 26: return 'Q'; break;
		case 27: return 'R'; break;
		case 28: return 'S'; break;
		case 29: return 'T'; break;
		case 30: return 'U'; break;
		case 31: return 'V'; break;
		case 32: return 'W'; break;
		case 33: return 'X'; break;
		case 34: return 'Y'; break;
		case 35: return 'Z'; break;
	}
	
	assert(!"n must satisfy n <= 35");
}

// 問題盤を表示
void printBoard(){

	cout << "*** BOARD ***" << endl;
	
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			Box* trgt_box = board->box(x,y);
			if(trgt_box->isTypeBlank()){
				cout << "+ ";
			}
			else{
				int trgt_num = trgt_box->getNumber();
				if(trgt_num < 10){
					cout << trgt_num << " ";
				}
				else{
					// アルファベット表示
					cout << changeIntToChar(trgt_num) << " ";
				}
			}
		}
		cout << endl;
	}
}

// 現状のラインを表示
void printLine(int i){

	cout << endl;
	cout << "print Line" << i << endl;
	Line* trgt_line = board->line(i);
	Point* trgt_track = trgt_line->getTrack();
	
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
	for (int i = 0; i < trgt_line->getLineLength(); i++) {
		Point p = trgt_track[i];
		int point_x = p.x;
		int point_y = p.y;
		for_print[point_y][point_x] = -2;
	}
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] == -2) cout << "@";
			else if(for_print[y][x] == -1) cout << "+";
			else{
				if(for_print[y][x] < 10){
					cout << for_print[y][x];
				}
				else{
					cout << changeIntToChar(for_print[y][x]);
				}
			}
		}
		cout << endl;
	}
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
		Point* trgt_track = trgt_line->getTrack();
		for (int j = 0; j < trgt_line->getLineLength(); j++) {
			Point p = trgt_track[j];
			int point_x = p.x;
			int point_y = p.y;
			for_print[point_y][point_x] = i;
		}
	}
	
	cout << "*** SOLUTION ***" << endl;
	for(int y=0;y<board->getSizeY();y++){
		for(int x=0;x<board->getSizeX();x++){
			if(for_print[y][x] < 0){
				// 線が引かれていないマス："00"表示
				cout << "00";
			}
			else{
				// その他(2桁表示)
				if(for_print[y][x] < 10){
					cout << "0" << for_print[y][x];
				}
				else
					cout << for_print[y][x];
			}
			if(x!=(board->getSizeX()-1)){
				cout << ",";
			}
		}
		cout << endl;
	}
}
