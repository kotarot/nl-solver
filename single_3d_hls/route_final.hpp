/**
 * route_final.hpp
 *
 * for Vivado HLS
 */

/*******************************************************/
/**  最終ルーティングに関する関数                     **/
/*******************************************************/

#ifndef __ROUTE_FINAL_HPP__
#define __ROUTE_FINAL_HPP__

#include "board.hpp"
#include "route.hpp"

// 最終ルーティング
bool final_routing(const ap_int<8> trgt_line_id, Board *board, ap_int<8> *output);
bool isInserted_3(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);
bool isInserted_4(ap_int<7> x, ap_int<7> y, ap_int<5> z, Board *board);

#endif
