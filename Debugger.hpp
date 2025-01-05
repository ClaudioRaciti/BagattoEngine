#pragma once
#include <cstdint>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "LookupTables.hpp"
#include "HashTable.hpp"

class Debugger
{
public:
    Debugger(std::string t_FEN) : m_lookup{LookupTables::getInstance()}, m_board{t_FEN}, m_TTable{1<<22} {}
    uint64_t getPerft(int t_depth);
private:
    bool isInCheck();
    bool isCheck();
    bool isSqAttacked(int t_square, int t_attackingSide);
private:
    const LookupTables &m_lookup;
    Board m_board;
    MoveGenerator m_generator;
    HashTable m_TTable;
};
