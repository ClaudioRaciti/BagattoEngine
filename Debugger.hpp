#pragma once
#include <cstdint>

#include "Board.hpp"
#include "MoveGenerator.hpp"
#include "MagicBitboards.hpp"
#include "TT.hpp"

class Debugger
{
public:
    Debugger(std::string tFEN) : mLookup{MagicBitboards::getInstance()}, mBoard{tFEN}, mTT{1} {}
    uint64_t getPerft(int tDepth);
    void changePos(std::string tFEN) {mBoard = Board(tFEN);}
private:
    bool isInCheck();
    bool isCheck();
    bool isSqAttacked(int tSquare, int tAttackingSide);
private:
    const MagicBitboards &mLookup;
    Board mBoard;
    MoveGenerator mMoveGenerator;
    TT mTT;
};
