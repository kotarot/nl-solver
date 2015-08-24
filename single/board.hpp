/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOARD_HPP__
#define __BOARD_HPP__

// マスと線の数の最大値
#define MAX_BOXES 64
#define MAX_LINES 256

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
	Box* boxes[MAX_BOXES][MAX_BOXES];	// マスの集合
	Line* lines[MAX_LINES];				// ラインの集合
};

#endif
