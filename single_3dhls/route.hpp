/**
 * route.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/**  ルーティングに関する関数                         **/
/*******************************************************/

#ifndef __ROUTE_HPP__
#define __ROUTE_HPP__

// ルーティング
bool routing(int trgt_line_id/*, bool debug_option*/, int penalty_T, int penalty_C, int penalty_V);
bool isInserted_1(int x,int y,int z, Board *board);
bool isInserted_2(int x,int y,int z, Board *board);
int countLine(int x,int y,int z, Board *board);

// 最終ルーティング
#if 0
bool final_routing(int trgt_line_id, bool debug_option);
bool isInserted_3(int x,int y,int z);
bool isInserted_4(int x,int y,int z);
#endif

#if 0
// 記録と抹消
void recordLine(int trgt_line_id);
void deleteLine(int trgt_line_id);
#endif

#endif
