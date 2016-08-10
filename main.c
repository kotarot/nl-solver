#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "WINDOWS_LINUX.h"
#include "numberlink.h"

char fold[] = "C:\\Users\\tawada\\Desktop\\160715ADC\\OLD\\ADC2015\\20X20\\";
char fold2[] = "C:\\Users\\tawada\\Desktop\\160715ADC\\OLD\\ADC2015\\17X20\\";

int main()
{
	char qfilename1[] = "Q07.txt";
	char afilename1[] = "AQ07.txt";
	char qfilename2[] = "Q13.txt";
	char afilename2[] = "AQ13.txt";
	char qfilename3[] = "QXX.txt";
	char afilename3[] = "AQXX.txt";
	char str[1024];
	char str2[1024];
	NL* old[3];
	old[0] = (NL*)calloc(1, sizeof(NL));
	old[1] = (NL*)calloc(1, sizeof(NL));
	old[2] = (NL*)calloc(1, sizeof(NL));
	sprintfs(str, "%s%s", fold, qfilename1);
	sprintfs(str2, "%s%s", fold, afilename1);
	readOLD_NL(old[0], str, str2);
	sprintfs(str, "%s%s", fold, qfilename2);
	sprintfs(str2, "%s%s", fold, afilename2);
	readOLD_NL(old[1], str, str2);
	sprintfs(str, "%s%s", fold, qfilename3);
	sprintfs(str2, "%s%s", fold, afilename3);
	readOLD_NL(old[2], str, str2);
	NL nl;
	mergeNL(old, 3, &nl);
	point p;
	int i;
	srand((unsigned int)time(NULL));
	for (i = 0; i < 1000; i++)
	{
		p.xyz[X] = rand() % nl.size[X];
		p.xyz[Y] = rand() % nl.size[Y];
		p.xyz[Z] = (rand() % 2)+1;
		pileVia(&nl, &p);
	}
	int r = rand()%100;
	sprintfs(str, "2016-08-10\\NL_Q%02d.txt", r);
	sprintfs(str2, "2016-08-10\\T99_A%02d.txt", r);
	writeNLQ(&nl, str);
	writeNLA(&nl, str2);
	return 0;
}


