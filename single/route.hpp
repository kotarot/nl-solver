/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__

bool routing(int trgt_line_id);
bool isInserted(int x,int y,int d);
bool isFixed(int x,int y,int d,int c,int num);
int countLineNum(int x,int y);

void recording(int trgt_line_id);
void deleting(int trgt_line_id);
bool routingSourceToI(int trgt_line_id);
bool routingIToSink(int trgt_line_id);

#endif
