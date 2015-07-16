/*******************************************************/
/** クラス定義 **/
/*******************************************************/

#ifndef __LINE_HPP__
#define __LINE_HPP__

// ラインクラス
class Line{
public:
	static const int NOT_USE = -1;

	Line(int _index):x_0(NOT_USE),y_0(NOT_USE),x_1(NOT_USE),y_1(NOT_USE),x_i(NOT_USE),y_i(NOT_USE),candidate(false),im(false){index=_index;track.clear();}
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
	
	int getLineLength(){return track.size();}
	
	vector<int>* getTrack(){return &track;}
	void pushIntToTrack(int t){track.push_back(t);}
	void clearTrack(){track.clear();}

private:
	int index;		// ライン番号
	int x_0, y_0;	// ソースポート
	int x_1, y_1;	// シンクポート
	int x_i, y_i;	// 中間ポート
	bool has_line;	// スタートとゴールが隣接してないか？
					// 隣接してない場合: ラインを持つので true
	bool candidate;	// 中間ポートを利用する候補か？
	bool im;		// 中間ポートを利用するか？
	vector<int> track;
};

#endif
