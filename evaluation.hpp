#pragma once
#include <cstdint>
#include "Board.hpp"

inline void mirror(int &square) {square = 56 - (8*(square/8)) + square%8;};
int pieceValue(int piece, int sideToMove, int gamePhase, int square);
int countMaterial(int piece, int sideToMove, int gamePhase, uint64_t bitBoard);
int evaluate(const Board &t_bitBoards, int gamePhase);
