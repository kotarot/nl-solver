/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __LINE_HPP__
#define __LINE_HPP__

// ラインの最大長
#define MAX_LINE_LENGTH 1024

// ラインクラス
class Line{
public:
	static const int NOT_USE = -1;

	Line(int _index):x_0(NOT_USE),y_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),x_i(NOT_USE),y_i(NOT_USE),candidate(false),im(false){
		index = _index;
		length = 0;
	}
	~Line();

	void setSourcePort(int x,int y){x_0=x;y_0=y;}
	int getSourceX(){return x_0;}
	int getSourceY(){return y_0;}
	
	void setSinkPort(int x,int y){x_1=x;y_1=y;}
	int getSinkX(){return x_1;}
	int getSinkY(){return y_1;}
	
	// スタートとゴールが隣接
	void setHasLine(bool _has_line){has_line = _has_line;}
	bool getHasLine(){return has_line;}
	
	// 中間ポート
	void setIntermediatePort(int x,int y){x_i=x;y_i=y;}
	int getInterX(){return x_i;}
	int getInterY(){return y_i;}
	
	void setCandidate(){candidate=true;}
	void setNonCandidate(){candidate=false;}
	bool isCandidate(){return candidate;}
	
	void setIntermediateUse(){im=true;}
	void setIntermediateUnuse(){im=false;}
	bool isIntermediateUsed(){return im;}
	
	void changePort(){	// ソースとシンクの交換
		int tmp_x = x_0;
		x_0 = x_1;
		x_1 = tmp_x;
		int tmp_y = y_0;
		y_0 = y_1;
		y_1 = tmp_y;
	}
	
	int getLineLength() const { return length; }
	Point* getTrack() { return track; }
	void pushPointToTrack(const Point p) { track[length] = p; length++; }
	//void setPointToTrack(const int idx, const Point p) { track[idx] = p; }
	//Point getPointToTrack(const int idx) const { return track[idx]; }
	void clearTrack() { length = 0; }

private:
	int index;		// ライン番号
	int x_0, y_0;	// ソースポート
	int x_1, y_1;	// シンクポート
	int x_i, y_i;	// 中間ポート
	bool has_line;	// スタートとゴールが隣接してないか？
					// 隣接してない場合: ラインを持つので true
	bool candidate;	// 中間ポートを利用する候補か？
	bool im;		// 中間ポートを利用するか？
	
	int length;
	Point track[MAX_LINE_LENGTH];
};

#endif
