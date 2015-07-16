/*******************************************************/
/**  クラス定義  ***************************************/
/*******************************************************/

#ifndef __BOX_HPP__
#define __BOX_HPP__

// ボックス（マス）クラス
class Box{
public:
	enum BoxType {TYPE_NULL,TYPE_NUMBER,TYPE_BLANK};
	enum FixType {ALL_FIXED,HALF_FIXED,NO_FIXED};
	static const int NOT_USE = -1;
	
	Box(int _x,int _y):type(TYPE_NULL),f_type(NO_FIXED),num(NOT_USE),north(0),south(0),east(0),west(0),f_north(false),f_south(false),f_east(false),f_west(false),self_count(0),candidate(false){x=_x;y=_y;}
	~Box();
	
	bool isTypeNumber() const{return (type==TYPE_NUMBER);}
	void setTypeNumber(){type=TYPE_NUMBER;}
	
	bool isTypeBlank() const{return (type==TYPE_BLANK);}
	void setTypeBlank(){type=TYPE_BLANK;}
	
	bool isTypeAllFixed() const{return (f_type==ALL_FIXED);}
	void setTypeAllFixed(){f_type=ALL_FIXED;}
	
	bool isTypeHalfFixed() const{return (f_type==HALF_FIXED);}
	void setTypeHalfFixed(){f_type=HALF_FIXED;}
	
	int getNumber(){return num;}
	void setNumber(int _num){num=_num;}
	
	int getX(){return x;}
	int getY(){return y;}
	
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
	
	void fixNorthLine(){f_north=true;}
	void fixEastLine(){f_east=true;}
	void fixSouthLine(){f_south=true;}
	void fixWestLine(){f_west=true;}
	
	bool isNorthLineFixed(){return f_north;}
	bool isEastLineFixed(){return f_east;}
	bool isSouthLineFixed(){return f_south;}
	bool isWestLineFixed(){return f_west;}
	
	void incrementSelfCount(){self_count++;}
	void resetSelfCount(){self_count=0;}
	int getSelfCount(){return self_count;}
	
	void setCandidate(){candidate=true;}
	void setNonCandidate(){candidate=false;}
	bool isCandidate(){return candidate;}
	

private:
	BoxType type;	// 数字マスか空白マス
	FixType f_type; // 固定マス
	int num;		// 数字マスの数字
	int x, y;		// 座標
	int north;		// 北向きに存在する線の数
	int south;		// 南向きに存在する線の数
	int east;		// 東向きに存在する線の数
	int west;		// 西向きに存在する線の数
	bool f_north;	// 北向きに固定線を持つ？
	bool f_south;	// 南向きに固定線を持つ？
	bool f_east;	// 東向きに固定線を持つ？
	bool f_west;	// 西向きに固定線を持つ？
	
	int self_count; // ライン不通過回数カウント用変数
	bool candidate; // 中間ポートの候補？
};

#endif
