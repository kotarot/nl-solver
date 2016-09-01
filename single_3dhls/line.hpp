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
		index = -1; track_index = 0; /*track.clear();*/ }
	Line(int _index):x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),via_specified(NOT_USE){
		index=_index; track_index = 0; /*track.clear();*/ }
	~Line() { ; }

	int getIndex(){return index;}
	void setIndex(int _index) { index = _index; }

	void setSourcePort(int x,int y,int z){x_0=x;y_0=y;z_0=z;}
	int getSourceX(){return x_0;}
	int getSourceY(){return y_0;}
	int getSourceZ(){return z_0;}

	void setSinkPort(int x,int y,int z){x_1=x;y_1=y;z_1=z;}
	int getSinkX(){return x_1;}
	int getSinkY(){return y_1;}
	int getSinkZ(){return z_1;}

	// スタートとゴールが隣接していないか？
	void setHasLine(bool _has_line){has_line = _has_line;}
	bool getHasLine(){return has_line;}

	// 指定ビア(任意)
	void setSpecifiedVia(int _via_specified){via_specified = _via_specified;}
	int getSpecifiedVia(){return via_specified;}

	void changePort(){ // ソースとシンクの交換
		int tmp_x = x_0;
		x_0 = x_1;
		x_1 = tmp_x;
		int tmp_y = y_0;
		y_0 = y_1;
		y_1 = tmp_y;
		int tmp_z = z_0;
		z_0 = z_1;
		z_1 = tmp_z;
	}

	//int getLineLength(){return track.size();}
	int getLineLength(){return track_index;}

	//vector<Point>* getTrack(){return &track;}
	//void pushPointToTrack(Point p){track.push_back(p);}
	//void clearTrack(){track.clear();}
	Point track[MAX_TRACKS];
	int track_index;

private:
	int index;         // ライン番号
	int x_0, y_0, z_0; // ソースポート
	int x_1, y_1, z_1; // シンクポート

	bool has_line; // スタートとゴールが隣接していないか？
	int via_specified; // 指定ビア(任意)
	//vector<Point> track;
};

#endif
