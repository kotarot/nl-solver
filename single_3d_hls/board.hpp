/**
 * board.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __BOARD_HPP__
#define __BOARD_HPP__

#include "./main.hpp"
#include "./box.hpp"
#include "./line.hpp"
#include "./via.hpp"

// ボードクラス
class Board{
public:
	Board(){;}
	Board(ap_int<7> _x, ap_int<7> _y, ap_int<6> _z, ap_int<8> _ln, ap_int<8> _vi){
		size_x = _x;
		size_y = _y;
		size_z = _z;
		line_num = _ln;
		via_num = _vi;

		for(ap_int<5> z=0;z<size_z;z++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=2
			for(ap_int<7> y=0;y<size_y;y++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
				for(ap_int<7> x=0;x<size_x;x++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
//#pragma HLS PIPELINE
					boxes[z][y][x].setX(x);
					boxes[z][y][x].setY(y);
					boxes[z][y][x].setZ(z);
				}
			}
		}
		for(ap_int<8> i=1;i<=line_num;i++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=90 avg=50
//#pragma HLS PIPELINE
			lines[i].setIndex(i);
		}
		for(ap_int<8> v=1;v<=via_num;v++){
#pragma HLS LOOP_TRIPCOUNT min=5 max=45 avg=25
//#pragma HLS PIPELINE
			vias[v].setIndex(v);
		}
	}
	~Board(){
		; // デストラクタ（要メモリ解放）
	}

	void init(ap_int<7> _x, ap_int<7> _y, ap_int<5> _z, ap_int<8> _ln, ap_int<8> _vi){
		size_x = _x;
		size_y = _y;
		size_z = _z;
		line_num = _ln;
		via_num = _vi;

		for(ap_int<5> z=0;z<size_z;z++){
#pragma HLS LOOP_TRIPCOUNT min=1 max=8 avg=2
			for(ap_int<7> y=0;y<size_y;y++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
				for(ap_int<7> x=0;x<size_x;x++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=40 avg=20
//#pragma HLS PIPELINE
					boxes[z][y][x].setX(x);
					boxes[z][y][x].setY(y);
					boxes[z][y][x].setZ(z);
				}
			}
		}
		for(ap_int<8> i=1;i<=line_num;i++){
#pragma HLS LOOP_TRIPCOUNT min=10 max=90 avg=50
//#pragma HLS PIPELINE
			lines[i].setIndex(i);
		}
		for(ap_int<8> v=1;v<=via_num;v++){
#pragma HLS LOOP_TRIPCOUNT min=5 max=45 avg=25
//#pragma HLS PIPELINE
			vias[v].setIndex(v);
		}
	}

	ap_int<7> getSizeX(){return size_x;}
	ap_int<7> getSizeY(){return size_y;}
	ap_int<5> getSizeZ(){return size_z;}
	ap_int<8> getLineNum(){return line_num;}
	ap_int<8> getViaNum(){return via_num;}

	Box* box(ap_int<7> x, ap_int<7> y, ap_int<5> z){return &(boxes[z][y][x]);} // マスの取得
	Line* line(ap_int<8> idx){return &(lines[idx]);}             // ラインの取得
	Via* via(ap_int<8> idx){return &(vias[idx]);}                // ビアの取得

private:
	ap_int<7> size_x;   // X方向サイズ
	ap_int<7> size_y;   // Y方向サイズ
	ap_int<5> size_z;   // Z方向サイズ
	ap_int<8> line_num; // ライン数
	ap_int<8> via_num;  // ビア数
	Box boxes[MAX_LAYER][MAX_BOXES][MAX_BOXES]; // マスの集合
	Line lines[MAX_LINES + 1];                  // ラインの集合
	Via vias[MAX_LINES + 1];                    // ビアの集合
};

#endif
