#include "Debugger.hpp"
#include <iostream>
#include "utils.hpp"


uint64_t Debugger::getPerft(int t_depth)
{
    uint64_t nodes = 0;

    uint64_t key = m_board.getHash();
    size_t index = key % m_size;
    if (m_hash[index].first == key) assert(m_board == m_hash[index].second);
    m_hash[index] = {key, m_board};

    if (t_depth == 0) 
        return 1ULL;
    auto moveList = m_generator.generateMoves(m_board);
    for (auto move : moveList) {
        m_board.makeMove(move);
        if(!isInCheck()){
            nodes += getPerft(t_depth - 1);
        }
        m_board.undoMove(move);
    }
    return nodes;
}


bool Debugger::isInCheck()
{
    int sideToMove = m_board.getSideToMove();
    int kingSquare = m_board.getKingSquare(1 - sideToMove);
    return isSqAttacked(kingSquare, sideToMove);
}

bool Debugger::isCheck()
{
    int sideToMove = m_board.getSideToMove();
    int kingSquare = m_board.getKingSquare( sideToMove);
    return isSqAttacked(kingSquare, 1 - sideToMove);
}

bool Debugger::isSqAttacked(int t_square, int t_attackingSide)
{    
    uint64_t occupied = m_board.getBitboard(white) | m_board.getBitboard(black);
    uint64_t pawnsSet = m_board.getBitboard(pawn) & m_board.getBitboard(t_attackingSide);
    if ((m_lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;


    for (int piece = knight; piece <= king; piece ++){
        uint64_t pieceSet = m_board.getBitboard(piece) & m_board.getBitboard(t_attackingSide);
        if((m_lookup.getAttacks(piece, t_square, occupied) & pieceSet) != 0) return true;
    } 

    return false;
}