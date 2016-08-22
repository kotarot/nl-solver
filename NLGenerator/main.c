#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "WINDOWS_LINUX.h"
#include "numberlink.h"

int main(int argc, char* argv[])
{
	int i;
	int num;
	char qfilename[8][1024];
	char afilename[8][1024];
	
	srand((unsigned int)time(NULL));
	
	if (argc < 2)
	{
		char filename[5][7] = { "76.txt", "84.txt", "87.txt", "95.txt", "96.txt"};
		num = (rand()%3)+2;
		for (i = 0; i < num; i++)
		{
			int tmp = rand() % 5;
			sprintfs(qfilename[i], "old/p%s", filename[tmp]);
			sprintfs(afilename[i], "old/a%s", filename[tmp]);
		}
	}
	else
	{
		num = argc - 1;
		for (i = 0; i < num; i+=2)
		{
			sprintfs(qfilename[i], "%s", argv[i]);
			sprintfs(afilename[i], "%s", argv[i+1]);
		}
	}
	NL* old[8];
	for (i = 0; i < num; i++)
	{
		old[i] = (NL*)calloc(1, sizeof(NL));
		readOLD_NL(old[i], qfilename[i], afilename[i]);
	}

	NL nl;
	mergeNL(old, num, &nl);
	point p;

	for (i = 0; i < 10000; i++)
	{
		p.xyz[X] = rand() % nl.size[X];
		p.xyz[Y] = rand() % nl.size[Y];
		p.xyz[Z] = (rand() % (num-1))+1;
		pileVia(&nl, &p);
	}

	//LINE数100未満
	checkNL(&nl);

	//配線最適化
	//未実装
	optMap(&nl);

	int r = rand()%1000;
	char str[1024];
	char ID[1024];
	//命名規則
	//XXXYYYZLLLVVV
	//XXX:X方向の大きさ
	//YYY:Y方向の大きさ
	//ZZ:Z方向の大きさ
	//LLL:ライン数
	//VVV:ビア数
	//RRR:乱数
	sprintfs(ID, "%02dX%02dX%01d-%03d-%03d-%03d", nl.size[X], nl.size[Y], nl.size[Z], nl.num_line,nl.num_via,r);
	
	sprintfs(str, "3D-problem/NL_Q%s.txt", ID);
	writeNLQ(&nl, str); 
	sprintfs(str, "3D-problem/T99_A%s.txt", ID);
	writeNLA(&nl, str);
	return 0;
}


