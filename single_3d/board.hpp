/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOARD_HPP__
#define __BOARD_HPP__

// 各種最大値
// ライン数は 36*36*8
#define MAX_BOXES 64
#define MAX_LAYER 8
#define MAX_LINES 5184

// ボードクラス
class Board{
public:
	Board(int _x,int _y,int _z,int _ln,int _vi){
		size_x = _x;
		size_y = _y;
		size_z = _z;
		line_num = _ln;
		via_num = _vi;

		for(int z=0;z<size_z;z++){
			for(int y=0;y<size_y;y++){
				for(int x=0;x<size_x;x++){
					Box* box;
					box = new Box(x,y,z);
					boxes[z][y][x] = box;
				}
			}
		}
		for(int i=1;i<=line_num;i++){
			Line* line;
			line = new Line(i);
			lines[i] = line;
		}
		for(int v=1;v<=via_num;v++){
			Via* via;
			via = new Via(v);
			vias[v] = via;
		}
	}
	~Board(){
		// デストラクタ（要メモリ解放）
	}
	
	int getSizeX(){return size_x;}
	int getSizeY(){return size_y;}
	int getSizeZ(){return size_z;}
	int getLineNum(){return line_num;}
	int getViaNum(){return via_num;}
	
	Box* box(int x,int y,int z){return boxes[z][y][x];} // マスの取得
	Line* line(int idx){return lines[idx];}             // ラインの取得
	Via* via(int idx){return vias[idx];}                // ビアの取得

private:
	int size_x;   // X方向サイズ
	int size_y;   // Y方向サイズ
	int size_z;   // Z方向サイズ
	int line_num; // ライン数
	int via_num;  // ビア数
	Box* boxes[MAX_LAYER][MAX_BOXES][MAX_BOXES]; // マスの集合
	Line* lines[MAX_LINES];                      // ラインの集合
	Via* vias[MAX_LINES];                        // ビアの集合
};

#endif
