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

#include "board.hpp"

// ルーティング
bool routing(ap_uint<7> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, /*Board *board,*/ ap_int<4> *output);
bool _routing(ap_uint<7> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, Board *board, ap_int<4> *output);
bool isInserted_1(int x,int y,int z, Board *board);
bool isInserted_2(int x,int y,int z, Board *board);
int countLine(int x,int y,int z, Board *board);

// 最終ルーティング
#if 0
bool final_routing(int trgt_line_id, bool debug_option);
bool isInserted_3(int x,int y,int z);
bool isInserted_4(int x,int y,int z);
#endif

// 記録と抹消
void recordLine(ap_uint<7> trgt_line_id, Board *board);
#if 0
void deleteLine(int trgt_line_id);
#endif

#endif
