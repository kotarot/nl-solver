/**
 * box.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/**  クラス定義  ***************************************/
/*******************************************************/

#ifndef __BOX_HPP__
#define __BOX_HPP__

enum BoxType {TYPE_NULL,TYPE_NUMBER,TYPE_VIA,TYPE_INTERVIA,TYPE_BLANK};

// マスクラス
class Box{
public:
//	static const int NOT_USE = -1;

	Box():type(TYPE_NULL),index(NOT_USE),north(0),south(0),east(0),west(0){ x = y = z = -1; }
	Box(ap_int<7> _x, ap_int<7> _y, ap_int<5> _z):type(TYPE_NULL),index(NOT_USE),north(0),south(0),east(0),west(0){x=_x;y=_y;z=_z;}
	~Box() { ; }

	bool isTypeNumber() const{return (type==TYPE_NUMBER);}
	void setTypeNumber(){type=TYPE_NUMBER;}

	bool isTypeVia() const{return (type==TYPE_VIA);}
	void setTypeVia(){type=TYPE_VIA;}

	bool isTypeInterVia() const{return (type==TYPE_INTERVIA);}
	void setTypeInterVia(){type=TYPE_INTERVIA;}

	bool isTypeBlank() const{return (type==TYPE_BLANK);}
	void setTypeBlank(){type=TYPE_BLANK;}

	ap_int<8> getIndex(){return index;}
	void setIndex(ap_int<8> _index){index=_index;}

	ap_int<7> getX(){return x;}
	ap_int<7> getY(){return y;}
	ap_int<5> getZ(){return z;}
	void setX(ap_int<7> _x){ x = _x; }
	void setY(ap_int<7> _y){ y = _y; }
	void setZ(ap_int<5> _z){ z = _z; }

	void incrementNorthNum(){north++;}
	void decrementNorthNum(){north--;}
	ap_int<8> getNorthNum(){return north;}

	void incrementEastNum(){east++;}
	void decrementEastNum(){east--;}
	ap_int<8> getEastNum(){return east;}

	void incrementSouthNum(){south++;}
	void decrementSouthNum(){south--;}
	ap_int<8> getSouthNum(){return south;}

	void incrementWestNum(){west++;}
	void decrementWestNum(){west--;}
	ap_int<8> getWestNum(){return west;}

private:
	BoxType type; // マスの種類（数字，ビア，ビア間，空白）
	ap_int<8> index;    // 数字，ビア（空白マスは－１）
	ap_int<7> x, y; ap_int<5> z;  // 座標
	ap_int<8> north;    // 北向きに存在する線の数
	ap_int<8> south;    // 南向きに存在する線の数
	ap_int<8> east;     // 東向きに存在する線の数
	ap_int<8> west;     // 西向きに存在する線の数
};

#endif
