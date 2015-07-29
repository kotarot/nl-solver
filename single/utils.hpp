#ifndef __UTILS_HPP__
#define __UTILS_HPP__

// ２桁の整数をアルファベットに変換（３５以下）
char changeIntToChar(int n);

// 問題盤を表示
void printBoard();

// 現状のラインを表示
void printLine(int i);

// ラインナンバーから色をマッピング
string getcolorescape(int n);

// 正解を表示（正解表示は数字２桁）
void printSolution();
// 正解を表示 (ファイル出力)
void printSolutionToFile(char *filename);

#endif
