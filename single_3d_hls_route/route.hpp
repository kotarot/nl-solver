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
bool routing(ap_int<8> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, /*Board *board, ap_int<8> *output,*/ ap_int<8> bbb[8][40][40]);
bool routing_proc(ap_int<8> trgt_line_id, ap_uint<4> penalty_T, ap_uint<4> penalty_C, ap_uint<4> penalty_V, Board *board, ap_int<8> *output);
void routing_arrange(Line *trgt_line);
bool isInserted_1(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);
bool isInserted_2(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);
int countLine(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);

// 最終ルーティング
#if 0
bool final_routing(int trgt_line_id, bool debug_option);
bool isInserted_3(int x,int y,int z);
bool isInserted_4(int x,int y,int z);
#endif

// 記録と抹消
void recordLine(ap_int<8> trgt_line_id, Board *board);
#if 0
void deleteLine(int trgt_line_id);
#endif

#endif
