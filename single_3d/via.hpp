/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __VIA_HPP__
#define __VIA_HPP__

// ビアクラス
class Via{
public:
//	static const int NOT_USE = -1;

	Via(int _index):x_0(NOT_USE),y_0(NOT_USE),z_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),z_1(NOT_USE),used_line_num(0){index=_index;}
	~Via();

	int getIndex(){return index;}

	void setSourcePort(int x,int y,int z){x_0=x;y_0=y;z_0=z;}
	int getSourceX(){return x_0;}
	int getSourceY(){return y_0;}
	int getSourceZ(){return z_0;}
	
	void setSinkPort(int x,int y,int z){x_1=x;y_1=y;z_1=z;}
	int getSinkX(){return x_1;}
	int getSinkY(){return y_1;}
	int getSinkZ(){return z_1;}
	
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
	
	void incrementUsedLineNum(){used_line_num++;}
	void decrementUsedLineNum(){used_line_num--;}
	int getUsedLineNum(){return used_line_num;}

private:
	int index;         // ビア番号
	int x_0, y_0, z_0; // ソースポート
	int x_1, y_1, z_1; // シンクポート
	int used_line_num; // 使用数
};

#endif
