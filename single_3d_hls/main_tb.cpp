/**
 * main_tb.cpp
 *
 * for Vivado HLS
 */

#ifndef SOFTWARE
#include <stdio.h>
#include <string.h>
#include <ap_int.h>
#endif

#ifdef SOFTWARE
#include "ap_int.h"
#else
#include <ap_int.h>
#endif

#include "main.hpp"


int main() {
	using namespace std;

	int size_x = 8, size_y = 5, size_z = 2;
	//int line_num = 3;
	//int via_num = 1;

	// テストデータ (マトリックス形式)
	ap_int<8> boardmat[MAX_LAYER][MAX_BOXES][MAX_BOXES];
	for (int z = 0; z < MAX_LAYER; z++)
		for (int y = 0; y < MAX_BOXES; y++)
			for (int x = 0; x < MAX_BOXES; x++)
				boardmat[z][y][x] = -1;
	for (int z = 0; z < size_z; z++)
		for (int y = 0; y < size_y; y++)
			for (int x = 0; x < size_x; x++)
				boardmat[z][y][x] = 0;
	// Line#1 (x=0, y=0, z=0) -- (x=7, y=4, z=0)
	boardmat[0][0][0] = 1; boardmat[0][4][7] = 1;
	// Line#2 (x=0, y=4, z=0) -- (x=7, y=0, z=1)
	boardmat[0][4][0] = 2; boardmat[1][0][7] = 2;
	// Line#3 (x=1, y=2, z=0) -- (x=2, y=2, z=0)
	boardmat[0][2][1] = 3; boardmat[0][2][2] = 3;
	// Via#a (x=4, y=2, z=0-1)
	boardmat[0][2][4] = 100; boardmat[1][2][4] = 100;

#if 0
	// テストデータ (文字列形式)
	
#endif

	// 初期ボード表示
	cout << endl;
	cout << "PROBLEM" << endl;
	cout << "========" << endl;
	for (int z = 0; z < size_z; z++) {
		cout << "LAYER " << (z + 1) << endl;
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				if (boardmat[z][y][x] < 100)
					cout << " " << boardmat[z][y][x];
				else
					cout << " X";
			}
			cout << endl;
		}
	}

	// ソルバ実行
	ap_int<8> status;
	bool result = nlsolver(boardmat, &status);
	if (result)
		cout << endl << "Test Passed!" << endl;
	else
		cout << endl << "Test Failed!" << endl;
	cout << "(status = " << status << ")" << endl << endl;

	// 解表示
	cout << endl;
	cout << "SOLUTION" << endl;
	cout << "========" << endl;
	for (int z = 0; z < size_z; z++) {
		cout << "LAYER " << (z + 1) << endl;
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				if (boardmat[z][y][x] < 100)
					cout << " " << boardmat[z][y][x];
				else
					cout << " X";
			}
			cout << endl;
		}
	}

	return 0;
}
