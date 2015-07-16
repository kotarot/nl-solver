/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOARD_HPP__
#define __BOARD_HPP__

// ボードクラス
class Board{
public:
	Board(int _x,int _y,int _ln){
		size_x = _x;
		size_y = _y;
		line_num = _ln;
		for(int y=0;y<size_y;y++){
			for(int x=0;x<size_x;x++){
				Box* box;
				box = new Box(x,y);
				boxes[y][x] = box;
			}
		}
		for(int i=1;i<=line_num;i++){
			Line* line;
			line = new Line(i);
			lines[i] = line;
		}
	}
	~Board(){
		// デストラクタ（要メモリ解放）
	}
	
	int getSizeX(){return size_x;}
	int getSizeY(){return size_y;}
	int getLineNum(){return line_num;}
	
	Box* box(int x,int y){return boxes[y][x];}	// マスの取得
	Line* line(int idx){return lines[idx];}		// ラインの取得
	
private:
	int size_x;		// X方向サイズ
	int size_y;		// Y方向サイズ
	int line_num;	// ライン数
	map<int,map<int,Box*> > boxes;	// マスの集合
	map<int,Line*> lines;	// ラインの集合
};

#endif
