#ifndef __UTILS_HPP__
#define __UTILS_HPP__

// 問題盤を表示
void printBoard();

// 正解を表示（正解表示は数字２桁）
void printSolution();
// 正解を表示 (ファイル出力)
void printSolutionToFile(char *filename);

// メルセンヌ・ツイスタ
void mt_init_genrand(unsigned long s);
unsigned long mt_genrand_int32(int a, int b);

#endif
