/**
 * via.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __VIA_HPP__
#define __VIA_HPP__

// ビアクラス
class Via{
public:
//	static const int NOT_USE = -1;

	Via():x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),used_line_num(0){ index = -1; }
	Via(ap_uint<7> _index):x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),used_line_num(0){index=_index;}
	~Via() { ; }

	ap_int<8> getIndex(){return index;}
	void setIndex(ap_int<8> _index) { index = _index; }

	void setSourcePort(ap_int<7> x, ap_int<7> y, ap_int<5> z){x_0=x;y_0=y;z_0=z;}
	ap_int<7> getSourceX(){return x_0;}
	ap_int<7> getSourceY(){return y_0;}
	ap_int<5> getSourceZ(){return z_0;}

	void setSinkPort(ap_int<7> x, ap_int<7> y, ap_int<5> z){x_1=x;y_1=y;z_1=z;}
	ap_uint<7> getSinkX(){return x_1;}
	ap_uint<7> getSinkY(){return y_1;}
	ap_uint<5> getSinkZ(){return z_1;}

	void changePort(){ // ソースとシンクの交換
		ap_int<7> tmp_x = x_0;
		x_0 = x_1;
		x_1 = tmp_x;
		ap_int<7> tmp_y = y_0;
		y_0 = y_1;
		y_1 = tmp_y;
		ap_int<7> tmp_z = z_0;
		z_0 = z_1;
		z_1 = tmp_z;
	}

	void incrementUsedLineNum(){used_line_num++;}
	void decrementUsedLineNum(){used_line_num--;}
	ap_int<8> getUsedLineNum(){return used_line_num;}

private:
	ap_int<8> index;         // ビア番号
	ap_int<7> x_0, y_0; ap_uint<5> z_0; // ソースポート
	ap_int<7> x_1, y_1; ap_uint<5> z_1; // シンクポート
	ap_int<8> used_line_num; // 使用数
};

#endif
