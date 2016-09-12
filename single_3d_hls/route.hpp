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

// ソース層に COST_MAX のコスト値が含まれている場合は引き継がない (issue #82)
#define COST_MAX 2047 //#define COST_MAX 32767

// ルーティング
bool routing(const ap_int<8> trgt_line_id, const ap_uint<12> penalty_T, const ap_uint<12> penalty_C, const ap_uint<12> penalty_V, Board *board, ap_int<8> *output);
void routing_arrange(Line *trgt_line);
bool isInserted_1(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);
bool isInserted_2(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);
int countLine(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);

// 記録と抹消
void recordLine(ap_int<8> trgt_line_id, Board *board);
void deleteLine(ap_int<8> trgt_line_id, Board *board);

#endif
