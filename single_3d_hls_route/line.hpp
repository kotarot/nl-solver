/**
 * line.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __LINE_HPP__
#define __LINE_HPP__

//#include <vector>
#include "./main.hpp"

// ラインクラス
class Line{
public:
//	static const int NOT_USE = -1;

	Line():x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),via_specified(NOT_USE){
		index = -1; track_index = 0; }
	Line(ap_int<8> _index):x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),via_specified(NOT_USE){
		index=_index; track_index = 0; }
	~Line() { ; }

	ap_int<8> getIndex(){return index;}
	void setIndex(ap_int<8> _index) { index = _index; }

	void setSourcePort(ap_int<7> x, ap_int<7> y, ap_int<5> z){x_0=x;y_0=y;z_0=z;}
	ap_int<7> getSourceX(){return x_0;}
	ap_int<7> getSourceY(){return y_0;}
	ap_int<5> getSourceZ(){return z_0;}

	void setSinkPort(ap_int<7> x, ap_int<7> y, ap_int<5> z){x_1=x;y_1=y;z_1=z;}
	ap_int<7> getSinkX(){return x_1;}
	ap_int<7> getSinkY(){return y_1;}
	ap_int<5> getSinkZ(){return z_1;}

	// スタートとゴールが隣接していないか？
	void setHasLine(bool _has_line){has_line = _has_line;}
	bool getHasLine(){return has_line;}

	// 指定ビア(任意)
	void setSpecifiedVia(ap_int<8> _via_specified){via_specified = _via_specified;}
	ap_int<8> getSpecifiedVia(){return via_specified;}

	void changePort(){ // ソースとシンクの交換
		ap_int<7> tmp_x = x_0;
		x_0 = x_1;
		x_1 = tmp_x;
		ap_int<7> tmp_y = y_0;
		y_0 = y_1;
		y_1 = tmp_y;
		ap_int<5> tmp_z = z_0;
		z_0 = z_1;
		z_1 = tmp_z;
	}

	ap_int<16> getLineLength(){return track_index;}

	Point track[MAX_TRACKS];
	ap_int<16> track_index;

private:
	ap_int<8> index;         // ライン番号
	ap_int<7> x_0, y_0; ap_int<5> z_0; // ソースポート
	ap_int<7> x_1, y_1; ap_int<5> z_1; // シンクポート

	bool has_line; // スタートとゴールが隣接していないか？
	ap_int<16> via_specified; // 指定ビア(任意)
};

#endif
