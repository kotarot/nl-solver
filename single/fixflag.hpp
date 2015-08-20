/*******************************************************/
/** 「空白マス非存在性」に基づきフラグセットする関数  **/
/*******************************************************/

#ifndef __FIXFLAG_HPP__
#define __FIXFLAG_HPP__

// Fix-Flag を自動で生成する
void generateFixFlag();

// 隣接数字を固定する
void fixAdjacentNum();

// Fix-Flag をファイルで与える
void setFixFlagFromFile(char *filename);

// Fix-Flag を表示する
void printFixFlag();

#endif
