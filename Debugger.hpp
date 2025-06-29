#pragma once
#include <vector>
#include <cstdint>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "MagicBitboards.hpp"
#include "TT.hpp"

class Debugger
{
public:
    Debugger(std::string t_FEN) : m_lookup{MagicBitboards::getInstance()}, m_board{t_FEN}, m_TTable{1} {}
    uint64_t getPerft(int t_depth);
    void changePos(std::string t_FEN) {m_board = Board(t_FEN);}
private:
    bool isInCheck();
    bool isCheck();
    bool isSqAttacked(int t_square, int t_attackingSide);
private:
    const MagicBitboards &m_lookup;
    Board m_board;
    MoveGenerator m_generator;
    TT m_TTable;
};
