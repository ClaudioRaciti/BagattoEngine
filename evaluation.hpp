#pragma once
#include <cstdint>
#include "Board.hpp"

inline void mirror(int &square) {square = 56 - (8*(square/8)) + square%8;};
int16_t evaluate(const Board &bitBoards);