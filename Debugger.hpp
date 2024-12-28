#pragma once
#include <cstdint>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "LookupTables.hpp"

class Debugger
{
public:
    Debugger() : m_lookup{LookupTables::getInstance()} {}
    uint64_t getPerft(int t_depth);
private:
    bool isInCheck();
    bool isSqAttacked(int t_square, int t_attackingSide);
private:
    Board m_board;
    MoveGenerator m_generator;
    const LookupTables &m_lookup;
};
