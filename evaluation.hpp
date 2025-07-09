#pragma once
#include <cstdint>
#include "Board.hpp"

inline void mirror(int &square) {square = 56 - (8*(square/8)) + square%8;};
int16_t pieceValue(int piece, int sideToMove, int gamePhase, int square);
int16_t countMaterial(int piece, int sideToMove, int gamePhase, uint64_t bitBoard);
int16_t evaluate(const Board &bitBoards, int gamePhase);
int gamePhase(int materialCount);
