/*******************************************************/
/**  クラス定義  ***************************************/
/*******************************************************/

#ifndef __BOX_HPP__
#define __BOX_HPP__

// マスクラス
class Box{
public:
	enum BoxType {TYPE_NULL,TYPE_NUMBER,TYPE_VIA,TYPE_INTERVIA,TYPE_BLANK};
//	static const int NOT_USE = -1;
	
	Box(int _x,int _y,int _z):type(TYPE_NULL),index(NOT_USE),north(0),south(0),east(0),west(0){x=_x;y=_y;z=_z;}
	~Box();
	
	bool isTypeNumber() const{return (type==TYPE_NUMBER);}
	void setTypeNumber(){type=TYPE_NUMBER;}
	
	bool isTypeVia() const{return (type==TYPE_VIA);}
	void setTypeVia(){type=TYPE_VIA;}
	
	bool isTypeInterVia() const{return (type==TYPE_INTERVIA);}
	void setTypeInterVia(){type=TYPE_INTERVIA;}

	bool isTypeBlank() const{return (type==TYPE_BLANK);}
	void setTypeBlank(){type=TYPE_BLANK;}
	
	int getIndex(){return index;}
	void setIndex(int _index){index=_index;}
	
	int getX(){return x;}
	int getY(){return y;}
	int getZ(){return z;}
	
	void incrementNorthNum(){north++;}
	void decrementNorthNum(){north--;}
	int getNorthNum(){return north;}
	
	void incrementEastNum(){east++;}
	void decrementEastNum(){east--;}
	int getEastNum(){return east;}
	
	void incrementSouthNum(){south++;}
	void decrementSouthNum(){south--;}
	int getSouthNum(){return south;}
	
	void incrementWestNum(){west++;}
	void decrementWestNum(){west--;}
	int getWestNum(){return west;}
	
private:
	BoxType type; // マスの種類（数字，ビア，ビア間，空白）
	int index;    // 数字，ビア（空白マスは－１）
	int x, y, z;  // 座標
	int north;    // 北向きに存在する線の数
	int south;    // 南向きに存在する線の数
	int east;     // 東向きに存在する線の数
	int west;     // 西向きに存在する線の数
};

#endif
