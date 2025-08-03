#include "Debugger.hpp"
#include <iostream>
#include "utils.hpp"


uint64_t Debugger::getPerft(int tDepth)
{
    uint64_t nodes = 0;

    if (tDepth == 0) 
        return 1ULL;

    std::vector<Move> moveList;
    moveList.reserve(256);
    if(isCheck()) mMoveGenerator.evadeChecks(mBoard, moveList);
    else mMoveGenerator.generate(mBoard, moveList);

    for (auto move : moveList) {
        mBoard.makeMove(move);
        if(!isInCheck()){
            nodes += getPerft(tDepth - 1);
        }
        mBoard.undoMove(move);
    }
    return nodes;
}


bool Debugger::isInCheck()
{
    int sideToMove = mBoard.getSideToMove();
    int kingSquare = mBoard.getKingSquare(1 - sideToMove);
    return isSqAttacked(kingSquare, sideToMove);
}

bool Debugger::isCheck()
{
    int sideToMove = mBoard.getSideToMove();
    int kingSquare = mBoard.getKingSquare( sideToMove);
    return isSqAttacked(kingSquare, 1 - sideToMove);
}

bool Debugger::isSqAttacked(int tSquare, int tAttackingSide)
{    
    uint64_t occupied = mBoard.getBitboard(white) | mBoard.getBitboard(black);
    uint64_t pawnsSet = mBoard.getBitboard(pawn) & mBoard.getBitboard(tAttackingSide);
    if ((mLookup.pawnAttacks(tSquare, 1-tAttackingSide) & pawnsSet) != 0) return true;


    for (int piece = knight; piece <= king; piece ++){
        uint64_t pieceSet = mBoard.getBitboard(piece) & mBoard.getBitboard(tAttackingSide);
        if((mLookup.getAttacks(piece, tSquare, occupied) & pieceSet) != 0) return true;
    } 

    return false;
}