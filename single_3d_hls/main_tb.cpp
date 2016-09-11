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

	int size_x = 10, size_y = 5, size_z = 3;
	int line_num = 4;
	int via_num = 2;

#if 0
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
#endif

	// テストデータ (文字列形式)
	//char boardstr[BOARDSTR_SIZE] = "X10Y05Z3L010000107041L020004107002L030102102021L040900100003V010402104022V020904309041";
	char boardstr[BOARDSTR_SIZE] = "X10Y18Z2L010900109002L020901105012L030902103052L040903103062L050904100102L060905106012L070906109022L080717109102L090808109112L100017209172L110401200072L120912208152L130009201092L140709209092L150901206052L160309204092L170701209072L180101201022L190011202152L200016202162V010800108002V020507105072V030207102072V040416104162V050311103112V060712107122V070104101042V080806108062V090812108122";
	//char boardstr[BOARDSTR_SIZE] = "X20Y36Z4L010917109172L021119111192L030715107152L040207108153L050720107202L060913109132L071501115012L081002110024L090419109273L100304103042L111514115142L121617113231L130911109112L140406104064L150510105102L161708117082L170202102022L181820118202L191207109033L201507101311L210625106252L221805104081L230605106054L241702117022L250826111311L260924116253L270911413043L280202409083L290510403203L300207202074L310419405293L321527312283L330830318204L341002213093L350419208253L361019309223L370304403103L381617215223L390427312074L400406204033L411125309244L421702412023L430720409193L440516305193L451207210033L461507215074L471223310273L481805214063L49060525063L501617416203L510826208264L520924202183L530202302073L540702303053L550820309174L561702315014L571218310223L581514414193L591615312173L601503314113L610604318054L621119413263L630805304083L641105307073L651429311303L661708417123L671207311103L681707317173L690718304223L701008309133L710609304133L720715407123L730626306313L740511309134L750212303153L761718316303L771212315133L780916314163L790625403253V010313203133V021033310334V031104311044V041609116092V051712117122V061528215283V070110201103V081100211003V091701217014V101023110232V111106211063V121330113302V130910309104V140514305144V151721117212V160017100173V171728117282V180723107232V191232312324V200318103184V210814108142V221110111102V231111311114V241418314184V251013210134V261713317134V270114101144V281422214223V291033110332V300621106212V310731107312V321114111142V330710307104V340428204283V351608316084V361726317264V371327313274V380220102202V390632106323V401129211294V411311213113V421603316034V431320113204V440220302204V450709107092V460431304314V470816308164V481128311284V491628116283V501015110153V510821308214V520722307224V530231302314";
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
	// Line#4 (x=9, y=0, z=0) -- (x=0, y=0, z=2)
	boardmat[0][0][9] = 4; boardmat[2][0][0] = 4;
	// Via#a (x=4, y=2, z=0-1)
	boardmat[0][2][4] = 100; boardmat[1][2][4] = 100;
	// Via#zz (x=9, y=4, z=0-2)
	boardmat[0][4][9] = 100; boardmat[1][4][9] = 100; boardmat[2][4][9] = 100;

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
	bool result = nlsolver(boardstr, &status);
	if (result)
		cout << endl << "Test Passed!" << endl;
	else
		cout << endl << "Test Failed!" << endl;
	cout << "(status = " << status << ")" << endl << endl;

	// 解表示
	int i = 0;
	cout << endl;
	cout << "SOLUTION" << endl;
	cout << "========" << endl;
	for (int z = 0; z < size_z; z++) {
		cout << "LAYER " << (z + 1) << endl;
		for (int y = 0; y < size_y; y++) {
			for (int x = 0; x < size_x; x++) {
				cout << " " << (int)(boardstr[i]);
				i++;
			}
			cout << endl;
		}
	}

	return 0;
}
