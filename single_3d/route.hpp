/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__

// ルーティング
bool routing(int trgt_line_id, bool debug_option);
bool isInserted_1(int x,int y,int z);
bool isInserted_2(int x,int y,int z);
int countLine(int x,int y,int z);

// 最終ルーティング
bool final_routing(int trgt_line_id, bool debug_option);
bool isInserted_3(int x,int y,int z);
bool isInserted_4(int x,int y,int z);

// 記録と抹消 
void recordLine(int trgt_line_id);
void deleteLine(int trgt_line_id);

#endif
