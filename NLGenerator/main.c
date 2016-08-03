#include <stdio.h>
#include <stdlib.h>
#include "WINDOWS_LINUX.h"
#include "numberlink.h"

char fold[] = "";
char fold2[] = "";
/*
int main()
{
	char qfilename1[] = "Q07.txt";
	char afilename1[] = "AQ07.txt";
	char qfilename2[] = "Q13.txt";
	char afilename2[] = "AQ13.txt";
	char str[1024];
	char str2[1024];
	NL* old[2];
	old[0] = (NL*)calloc(1, sizeof(NL));
	old[1] = (NL*)calloc(1, sizeof(NL));
	sprintfs(str, "%s%s", fold, qfilename1);
	sprintfs(str2, "%s%s", fold, afilename1);
	readOLD_NL(old[0], str, str2);
	sprintfs(str, "%s%s", fold, qfilename2);
	sprintfs(str2, "%s%s", fold, afilename2);
	readOLD_NL(old[1], str, str2);
	NL nl;
	mergeNL(old, 2, &nl);
	point p;
	int i;
	srand(time(NULL));
	for (i = 0; i < 1000; i++)
	{
		p.xyz[X] = rand() % nl.size[X];
		p.xyz[Y] = rand() % nl.size[Y];
		p.xyz[Z] = 1;
		pileVia(&nl, &p);
	}
	int r = rand();
	sprintfs(str, "2016-08-02\\Q%03d.txt", r);
	sprintfs(str2, "2016-08-02\\A%03d.txt", r);
	writeNLQ(&nl, str);
	writeNLA(&nl, str2);
	return 0;
}
*/

int main()
{
	char qfilename1[] = "NL_Q01.txt";
	char afilename1[] = "T99_A01.txt";
	char qfilename2[] = "NL_Q02.txt";
	char afilename2[] = "T99_A02.txt";
	char str[1024];
	char str2[1024];
	NL* old[2];
	old[0] = (NL*)calloc(1, sizeof(NL));
	old[1] = (NL*)calloc(1, sizeof(NL));
	sprintfs(str, "%s%s", fold2, qfilename1);
	sprintfs(str2, "%s%s", fold2, afilename1);
	readOLD_NL(old[0], str, str2);
	sprintfs(str, "%s%s", fold2, qfilename2);
	sprintfs(str2, "%s%s", fold2, afilename2);
	readOLD_NL(old[1], str, str2);
	NL nl;
	mergeNL(old, 2, &nl);
	point p;
	int i;
	srand(time(NULL));
	for (i = 0; i < 1000; i++)
	{
		p.xyz[X] = rand() % nl.size[X];
		p.xyz[Y] = rand() % nl.size[Y];
		p.xyz[Z] = 1;
		pileVia(&nl, &p);
	}
	int r = rand()%100;
	sprintfs(str, "2016-08-02\\NL_Q%02d.txt", r);
	sprintfs(str2, "2016-08-02\\T99_A%02d.txt", r);
	writeNLQ(&nl, str);
	writeNLA(&nl, str2);
	return 0;
}


