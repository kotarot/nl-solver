#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "numberlink.h"
#include "WINDOWS_LINUX.h"

//NLデータ構造の初期化
void initNL(NL* nl)
{
	nl->size[X] = 0;
	nl->size[Y] = 0;
	nl->size[Z] = 1;
	nl->num_line = 0;
	nl->num_via = 0;
	nl->map = NULL;
}
//
void readOLD_NL(NL* nl, char* qfile, char* afile)
{
	initNL(nl);
	readOLD_NLQ(nl, qfile);
	readOLD_NLA(nl, afile);
}

void readOLD_NLQ(NL* nl, char* filename)
{
	char str[1024];
	FILE* fp;
	int i;

	fopens(&fp, filename, "r");
	fgets(str, sizeof(str), fp);
	sscanfs(str, "SIZE %dX%d", nl->size + X, nl->size + Y);
	fgets(str, sizeof(str), fp);
	sscanfs(str, "LINE_NUM %d", &(nl->num_line));
	//nl->lines = (line*)calloc(nl->num_line, sizeof(line));
	for (i = 0; i < nl->num_line; i++)
	{
		int tmp;
		fgets(str, sizeof(str), fp);
		sscanfs(str, "LINE#%d (%d,%d)-(%d,%d)\n", &tmp,
			nl->lines[i].p[SRC].xyz + X, nl->lines[i].p[SRC].xyz + Y,
			nl->lines[i].p[DST].xyz + X, nl->lines[i].p[DST].xyz + Y);
		nl->lines[i].p[SRC].xyz[Z] = 0;
		nl->lines[i].p[DST].xyz[Z] = 0;
		int2val(tmp, &(nl->lines[i].name));
	}
	fclose(fp);
}
val* getVal(NL* nl, point* p)
{
	return nl->map + ((p->xyz[Z] * nl->size[Y]) + p->xyz[Y])*nl->size[X] + p->xyz[X];
}
val* getValXYZ(NL* nl, int x, int y, int z)
{
	return nl->map + ((z*nl->size[Y]) + y)*nl->size[X] + x;
}
void cpVal(val* src, val* dst)
{
	dst->value[0] = src->value[0];
	dst->value[1] = src->value[1];
}
void cpPoint(point* src, point* dst)
{
	dst->xyz[X] = src->xyz[X];
	dst->xyz[Y] = src->xyz[Y];
	dst->xyz[Z] = src->xyz[Z];
}
void cpLine(line* src, line* dst)
{
	cpVal(&src->name, &dst->name);
	cpPoint(src->p + SRC, dst->p + SRC);
	cpPoint(src->p + DST, dst->p + DST);
}
void readOLD_NLA(NL* nl, char* filename)
{
	char str[1024];
	FILE* fp;
	int i, j;
	fopens(&fp, filename, "r");
	fgets(str, sizeof(str), fp);
	sscanfs(str, "SIZE %dX%d\n", nl->size + X, nl->size + Y);
	nl->map = (val*)calloc(nl->size[X] * nl->size[Y], sizeof(val));
	for (i = 0; i < nl->size[Y]; i++)
	{
		char* t_str = str;
		fgets(str, sizeof(str), fp);
		for (j = 0; j < nl->size[X]; j++)
		{
			cpVal((val*)t_str, getValXYZ(nl, j, i, 0));
			t_str += 3;
		}
	}
	fclose(fp);
}
void int2val(int src, val* dst)
{
	dst->value[0] = src / 10 + '0';
	dst->value[1] = src % 10 + '0';
}
int val2int(val* dst)
{
	return (dst->value[0] - '0') * 10 + (dst->value[1] - '0');
}

void mergeNL(NL* srcs[], int n_src, NL* dst)
{
	int i, j, k;
	//サイズが同じかチェックしないといけない（未実装）
	dst->size[X] = srcs[0]->size[X];
	dst->size[Y] = srcs[0]->size[Y];
	dst->size[Z] = n_src;

	//総ライン数カウント
	dst->num_line = 0;
	for (i = 0; i < n_src; i++)
	{
		dst->num_line += srcs[i]->num_line;
	}

	//dst->lines = (line*)calloc(dst->num_line, sizeof(line));
	dst->num_via = 0;
	////ビア最大数はライン数以下なので，最大値で領域を確保する
	//dst->vias = (via*)calloc(dst->num_line, sizeof(via));
	dst->map = (val*)calloc(dst->size[X] * dst->size[Y] * dst->size[Z], sizeof(val));

	int index = 0;
	//ライン，地図のコピー
	for (k = 0; k < n_src; k++)
	{
		//第k層目では(index+1)番目から(index+srcs[k]->num_line)番目までのラインを使う
		for (j = 0; j < srcs[k]->num_line; j++)
		{
			int2val(index + (j + 1), &(dst->lines[index + j].name));
			cpPoint(srcs[k]->lines[j].p + SRC, dst->lines[index + j].p + SRC);
			cpPoint(srcs[k]->lines[j].p + DST, dst->lines[index + j].p + DST);
			dst->lines[index + j].p[SRC].xyz[Z] = k;
			dst->lines[index + j].p[DST].xyz[Z] = k;
		}
		for (i = 0; i < dst->size[Y]; i++)
			for (j = 0; j < dst->size[X]; j++)
			{
			int tmp = val2int(getValXYZ(srcs[k], j, i, 0));
			if (tmp != 0)tmp += index;
			int2val(tmp, getValXYZ(dst, j, i, k));
			}
		index += srcs[k]->num_line;
	}

}

//nlのpを端点とするビアを作ろうとする
//候補が2種類あるときはどちらか（未実装）
//いまは低い階層の方向にビアを作れないか試す
int pileVia(NL* nl, point* p)
{
	int k;
	if (nl->num_line == nl->num_via)return 0;
	val* t = getVal(nl, p);

	//ビアの端点がライン上にない
	if (t->value[0] == '0' && t->value[1] == '0')return 0;

	//そのラインがビアとつながっている
	//=そのラインの端点が異なる階層にある
	line* l = nl->lines + (val2int(t) - 1);
	if (l->p[SRC].xyz[Z] != l->p[DST].xyz[Z])return 0;

	//ラインの端点である
	if (equalsPoint(l->p + SRC, p))return 0;
	if (equalsPoint(l->p + DST, p))return 0;

	//低い階層の方向にビアを作れないか試す
	for (k = p->xyz[Z]-1; k >= 0; k--)
	{
		point p2;
		p2.xyz[X] = p->xyz[X];
		p2.xyz[Y] = p->xyz[Y];
		p2.xyz[Z] = k;

		val* d = getVal(nl, &p2);
		//ビアの端点がライン上にない
		//ビアを伸ばす余地がある
		if (d->value[0] == '0' && d->value[1] == '0')continue;

		//そのラインがビアとつながっている
		//=そのラインの端点が異なる階層にある
		line* l2 = nl->lines + (val2int(d) - 1);
		if (l2->p[SRC].xyz[Z] != l2->p[DST].xyz[Z])break;

		//ラインの端点である
		if (equalsPoint(l2->p + SRC, &p2))break;
		if (equalsPoint(l2->p + DST, &p2))break;

		//ラインl上の点pとラインll上の点ppをビアにしたい
		pileVia_sub(nl, l, p, l2, &p2);
		return 1;
	}

	return 0;
}

//導火線に火をつけるようにmapからラインを消す
void ignite(NL* nl, point* src, point* dst, val* v, val* zero)
{
	point p_now;
	cpPoint(src, &p_now);
	while (1)
	{
		if (equalsPoint(&p_now, dst))break;
		cpVal(zero, getVal(nl, &p_now));
		if (p_now.xyz[X] >= 1)
		{
			p_now.xyz[X]--;
			if (equalsVal(v, getVal(nl, &p_now)))continue;
			p_now.xyz[X]++;
		}
		if (p_now.xyz[X] < nl->size[X] - 1)
		{
			p_now.xyz[X]++;
			if (equalsVal(v, getVal(nl, &p_now)))continue;
			p_now.xyz[X]--;
		}
		if (p_now.xyz[Y] >= 1)
		{
			p_now.xyz[Y]--;
			if (equalsVal(v, getVal(nl, &p_now)))continue;
			p_now.xyz[Y]++;
		}
		if (p_now.xyz[Y] < nl->size[Y] - 1)
		{
			p_now.xyz[Y]++;
			if (equalsVal(v, getVal(nl, &p_now)))continue;
			p_now.xyz[Y]--;
		}
		printf("ERROR\n");
		exit(1);
	}
}

void pileVia_sub(NL* nl, line* l, point* p, line* l2, point* p2)
{
	int i,j,k;
	//選択は未実装
	//l->p[SRC]を使う
	//l->p[DST]は消す
	val v_now;
	cpVal(&l->name, &v_now);
	val zero;
	zero.value[0] = '0';
	zero.value[1] = '0';
	//l->p[DST]からpまでの経路をzeroにする
	ignite(nl, l->p + DST, p, &v_now,&zero);

	//選択は未実装
	//l2->p[SRC]を使う
	//l2->p[DST]は消す
	cpVal(&l2->name, &v_now);
	//l2->p[DST]からpまでの経路をzeroにする
	ignite(nl, l2->p + DST, p2, &v_now, &zero);

	//ビアを作る
	{
		int min;
		int max;
		nl->num_via++;
		nl->vias[nl->num_via - 1].name.value[0] = ((nl->num_via-1) / ('z' - 'a'+1)) + 'a'-1;
		nl->vias[nl->num_via - 1].name.value[1] = ((nl->num_via-1) % ('z' - 'a'+1)) + 'a';
		if (nl->vias[nl->num_via - 1].name.value[0] == 'a' - 1)
			nl->vias[nl->num_via - 1].name.value[0] = ' ';
		point px;
		if (p->xyz[Z]<p2->xyz[Z])
		{ 
			min = p->xyz[Z];
			max = p2->xyz[Z];
			cpPoint(p, &px);
		}
		else
		{
			min = p2->xyz[Z];
			max = p->xyz[Z];
			cpPoint(p2, &px);
		}
		nl->vias[nl->num_via - 1].length = max - min + 1;
		for (k = 0; k < nl->vias[nl->num_via - 1].length; k++)
		{
			cpPoint(&px,nl->vias[nl->num_via - 1].p+k);
			px.xyz[Z]++;
		}
	}
	
	//ラインを消す，
	{
		val cmp[2];
		val writer[2];
		if (val2int(&l->name) < val2int(&l2->name))
		{
			cpVal(&l2->name, cmp);
			cpVal(&l->name, writer);
			int2val(nl->num_line, cmp+1);
			cpVal(&l2->name, writer+1);
			cpPoint(l2->p + SRC, l->p + DST);
			if (!equalsVal(cmp,cmp+1))
			{
				cpLine(nl->lines + (nl->num_line - 1),l2);
				cpVal(cmp, &l2->name);
			}
		}
		else
		{
			cpVal(&l->name, cmp);
			cpVal(&l2->name, writer);
			int2val(nl->num_line, cmp + 1);
			cpVal(&l->name, writer + 1);
			cpPoint(l->p + SRC, l2->p + DST);
			if (!equalsVal(cmp, cmp + 1))
			{
				cpLine(nl->lines + (nl->num_line - 1), l);
				cpVal(cmp,&l->name);
			}
		}
		for (k = 0; k < nl->size[Z]; k++)
			for (i = 0; i < nl->size[Y]; i++)
				for (j = 0; j < nl->size[X]; j++)
				{
			val* v = getValXYZ(nl, j, i, k);
			if (equalsVal(v, cmp))
				cpVal(writer, v);
			else if (equalsVal(v, cmp+1))
				cpVal(writer+1, v);
				}
		nl->num_line--;
	}
	//ビアを地図に書き入れる
	for (k = 1; k < nl->vias[nl->num_via - 1].length-1; k++)
	{
		cpVal(getVal(nl, p), getValXYZ(nl,p->xyz[X], p->xyz[Y], k));
	}

}
void writeNLQ(NL* nl, char* filename)
{
	int i,j;
	FILE* fp;

	fopens(&fp, filename, "w");
	fprintfs(fp, "SIZE %dX%dX%d\n", nl->size[X], nl->size[Y], nl->size[Z]);
	fprintfs(fp, "LINE_NUM %d\n", nl->num_line);
	for (i = 0; i < nl->num_line; i++)
	{
		fprintfs(fp, "LINE#%d (%01d,%01d,%01d) (%01d,%01d,%01d)\n", val2int(&(nl->lines[i].name)),
			nl->lines[i].p[SRC].xyz[X], nl->lines[i].p[SRC].xyz[Y], nl->lines[i].p[SRC].xyz[Z]+1,
			nl->lines[i].p[DST].xyz[X], nl->lines[i].p[DST].xyz[Y], nl->lines[i].p[DST].xyz[Z]+1);
	}
	//fprintfs(fp, "VIA_NUM %d\n", nl->num_via);
	fprintfs(fp, "\n");
	for (i = 0; i < nl->num_via; i++)
	{
		fprintfs(fp, "VIA#");
		printVal(fp, &nl->vias[i].name);
		for (j = 0; j < nl->vias[i].length; j++)
			fprintfs(fp, " (%01d,%01d,%01d)",
			nl->vias[i].p[j].xyz[X], nl->vias[i].p[j].xyz[Y], nl->vias[i].p[j].xyz[Z]+1);
		fprintfs(fp, "\n");
	}
	fclose(fp);
}
void writeNLA(NL* nl, char* filename)
{
	int i, j, k;
	FILE* fp;
	fopens(&fp, filename, "w");
	fprintfs(fp, "SIZE %dX%dX%d\n", nl->size[X], nl->size[Y], nl->size[Z]);
	for (k = 0; k < nl->size[Z]; k++)
	{
		fprintfs(fp, "LAYER %d\n", (k+1));
		for (i = 0; i < nl->size[Y]; i++)
		{
			for (j = 0; j < nl->size[X]; j++)
			{
				printVal(fp, getValXYZ(nl, j, i, k));
				if (j != nl->size[X] - 1)fprintfs(fp, ",");
			}
			fprintfs(fp, "\n");
		}
		fprintfs(fp, "\n");
	}
	fclose(fp);
}
int equalsVal(val* src, val* dst)
{
	return (src->value[0] == dst->value[0]) && (src->value[1] == dst->value[1]);
}
int equalsPoint(point* src, point* dst)
{
	return (src->xyz[X] == dst->xyz[X]) && (src->xyz[Y] == dst->xyz[Y]) && (src->xyz[Z] == dst->xyz[Z]);
}
void printVal(FILE* fp, val* src)
{
	if (src->value[0] == ' ')
		fprintfs(fp, "%c", src->value[1]);
	else
		fprintfs(fp, "%c%c", src->value[0], src->value[1]);
}

void delLine(NL* nl, line* l)
{
	int i, j, k;
	val v;
	val last;
	val zero;
	cpVal(&l->name, &v);
	cpVal(&nl->lines[nl->num_line-1].name, &last);
	zero.value[0] = '0';
	zero.value[1] = '0';
	for (k = 0; k < nl->size[Z]; k++)
		for (i = 0; i < nl->size[Y]; i++)
			for (j = 0; j < nl->size[X]; j++)
			{
		point p;
		p.xyz[X] = j;
		p.xyz[Y] = i;
		p.xyz[Z] = k;
		val* now = getVal(nl, &p);
		if (equalsVal(now, &v))cpVal(&zero, now);
		if (equalsVal(now, &last))cpVal(&v, now);
			}
	
	cpLine(nl->lines + nl->num_line - 1, l);
	cpVal(&v, &l->name);
	nl->num_line--;
}
void checkNL(NL* nl)
{
	while (nl->num_line >= 100)
	{
		int index=rand() % nl->num_line;
		delLine(nl, nl->lines+index);
	}
}
int isEdge(NL* nl, int x, int y, int z)
{
	val* v=getValXYZ(nl,x,y,z);
	val* tmp;
	int f = 0;
	if (x == 0)f++;
	else
	{
		tmp = getValXYZ(nl, x - 1, y, z);
		if (equalsVal(v, tmp))f++;
	}
	if (x == nl->size[X]-1)f++;
	else
	{
		tmp = getValXYZ(nl, x + 1, y, z);
		if (equalsVal(v, tmp))f++;
	}
	if (y == 0)f++;
	else
	{
		tmp = getValXYZ(nl, x, y-1, z);
		if (equalsVal(v, tmp))f++;
	}
	if (y == nl->size[Y] - 1)f++;
	else
	{
		tmp = getValXYZ(nl, x, y+1, z);
		if (equalsVal(v, tmp))f++;
	}
	return f == 4 ? 1 : 0;
}
void optMap_sub(NL* nl)
{
	int i,j;
	point p,q;
	p.xyz[X] = rand() % nl->size[X];
	p.xyz[Y] = rand() % nl->size[Y];
	p.xyz[Z] = rand() % nl->size[Z];
	q.xyz[X] = -1;
	val v;
	val zero;
	cpVal(getVal(nl, &p),&v);
	zero.value[0] = '0';
	zero.value[1] = '0';
	if (equalsVal(&v, &zero))return;
	
	int* map;
	map = (int*)calloc(nl->size[X] * nl->size[Y]*4, sizeof(int));
	for (i = 0; i < nl->size[Y];i++)
		for (j = 0; j < nl->size[X]; j++)
		{
		map[(i*nl->size[X] + j) * 4] = 0;
		map[(i*nl->size[X] + j) * 4 + 1] = 0;
		map[(i*nl->size[X] + j) * 4 + 2] = 0;
		map[(i*nl->size[X] + j) * 4 + 3] = 0;
		if (isEdge(nl, j, i, p.xyz[Z]))
		{
			if (q.xyz[X] == -1)
			{
				q.xyz[X] = j;
				q.xyz[Y] = i;
				q.xyz[Z] = p.xyz[Z];
			}
			else
			{
				p.xyz[X] = j;
				p.xyz[Y] = i;
			}
		}
		}
	//ここまでで
	//端点はp,q
	//(ラインの端なのかビアなのかは区別しない)
	//
	//未実装





	free(map);
}
void optMap(NL* nl)
{
	int i;
	for(i=0;i<10000;i++)
		optMap_sub(nl);
}