#ifndef _NUMBERLINK_H
#define _NUMBERLINK_H

#define X (0)
#define Y (1)
#define Z (2)

#define WIDTH (0)
#define HEIGHT (1)
#define DEPTH (2)

#define SRC (0)
#define DST (1)

//���C����r�A��char2�����Ȃ̂�val�Ƃ����^���ň���
typedef struct{
	unsigned char value[2];
} val;

//���W (X,Y,Z)
typedef struct{
	int xyz[3];
} point;

//���C�� (BLACK,WHITE)
typedef struct{
	val name;
	point p[2];
}line;

//�r�A
typedef struct{
	val name;
	point p[8];
	int length;
}via;

//NL�̖��Ɖ𓚂��܂Ƃ߂�NL�Ƃ����f�[�^�\���ň���
//���Ȃ��size, num_line, num_via, lines, vias���g�p
//�𓚂Ȃ��size, map���g�p
typedef struct
{
	int size[3];
	int num_line;
	int num_via;
	line* lines;
	via* vias;
	val* map;
}NL;

unsigned char* getXY(NL*, int, int);
void initNL(NL*);
void readOLD_NLQ(NL*, char*);
void readOLD_NLA(NL*, char*);
void readOLD_NL(NL*, char*, char*);
void mergeNL(NL* srcs[], int n_src, NL* dst_new);
void setVia(NL* nl, int x, int y, int z0, int z1);
void writeNLQ(NL*, char*);
void writeNLA(NL*, char*);
int pileVia(NL* nl, point* p);
void pileVia_sub(NL* nl, line* l, point* p, line* l2, point* p2);

void cpVal(val* src, val* dst);
void cpPoint(point* src, point* dst);
int equalsVal(val*, val*);
int equalsPoint(point* src, point* dst);
void cpLine(line* src, line* dst);

val* getVal(NL* nl, point* p);
val* getValXYZ(NL* nl, int x, int y, int z);
void int2val(int src, val* dst);
int val2int(val* dst);
void printVal(FILE* fp, val* src);
#endif