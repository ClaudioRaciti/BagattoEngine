#include "MoveGenerator.hpp"
#include "utils.hpp"

MoveGenerator::MoveGenerator() : m_lookup{LookupTables::getInstance()}{
}

std::vector<Move> MoveGenerator::generateQuiets(const Board &t_board)
{
    std::vector<Move> moveList;
    moveList.reserve(256); // over the maximum number of moves possible for any legal position
    generatePawnsQuiets(moveList, t_board);
    generateCatstle(moveList, t_board);
    for (int pieceType = knight; pieceType <= king; pieceType ++)
        generatePieceQuiets(pieceType, moveList, t_board);
    return moveList;
}

void MoveGenerator::generatePieceQuiets(int t_piece, std::vector<Move> &t_moveList, const Board &t_board)
{
    uint64_t pieceSet = t_board.getBitboard(t_piece);
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t emptySet = ~occupied;

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = m_lookup.getAttacks(t_piece, occupied, startingSquare);

        uint64_t quietMoves = attackSet & emptySet;
        if (quietMoves) do {
            int endSquare = bitScanForward(quietMoves);
            t_moveList.emplace_back(Move(startingSquare, endSquare, quiet, t_piece));
        } while (quietMoves &= (quietMoves - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsQuiets(std::vector<Move> &t_moveList, const Board &t_board)
{
    static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
    static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
    static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);
    static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
    uint64_t emptySet = ~(t_board.getBitboard(white) | t_board.getBitboard(black));
    uint64_t pawnSet, doublePushSet, pushSet, promoSet;
    int offset;

    if(t_board.getSideToMove() == white) {
        pawnSet = t_board.getBitboard(pawn) | t_board.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4;
        pushSet = (pawnSet << 8 & emptySet) & ~rank8;
        promoSet = (pawnSet << 8 & emptySet) & rank8; 
        offset = -8;
    }
    else {
        pawnSet = t_board.getBitboard(pawn) | t_board.getBitboard(black);
        doublePushSet = (pawnSet >> 8) & emptySet;
        doublePushSet = (doublePushSet >> 8) & emptySet & rank5;
        pushSet = (pawnSet >> 8 & emptySet) & ~rank1;
        promoSet = (pawnSet >> 8 & emptySet) & rank1; 
        offset = 8;
    }

    if (pushSet) do {
        int endSq = bitScanForward(pushSet);
        int startSq = endSq + offset;
        t_moveList.emplace_back(Move(startSq, endSq, quiet, pawn));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = bitScanForward(promoSet);
        int startSq = endSq + offset;
        t_moveList.emplace_back(Move(startSq, endSq, knightPromo, pawn));
        t_moveList.emplace_back(Move(startSq, endSq, bishopPromo, pawn));
        t_moveList.emplace_back(Move(startSq, endSq, rookPromo, pawn));
        t_moveList.emplace_back(Move(startSq, endSq, queenPromo, pawn));
    } while (promoSet &= (promoSet - 1));

    if (doublePushSet) do {
        int endSq = bitScanForward(doublePushSet);
        int startSq = endSq + (2 * offset);
        t_moveList.emplace_back(Move(startSq, endSq, doublePush, pawn));
    } while (doublePushSet  &= (doublePushSet - 1));
}

void MoveGenerator::generateCatstle(std::vector<Move> &t_moveList, const Board &t_board)
{
    uint64_t emptySet = ~(t_board.getBitboard(black) | t_board.getBitboard(white));

    if (t_board.getSideToMove() == white){
        static constexpr uint64_t longCastleSquares = (uint64_t) 0x000000000000000e;
        static constexpr uint64_t shortCastleSquares = (uint64_t) 0x0000000000000060;
        uint64_t intersection = longCastleSquares & emptySet;
        if(
            t_board.getLongCastle(white) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(t_board, c1, black) &&
            ! isSquareAttacked(t_board, d1, black) &&
            ! isSquareAttacked(t_board, e1, black)
        ) t_moveList.emplace_back(Move(e1, c1, queenCastle, king));
        
        intersection = shortCastleSquares & emptySet; 
        if(
            t_board.getShortCastle(white) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(t_board, e1, black) &&
            ! isSquareAttacked(t_board, f1, black) &&
            ! isSquareAttacked(t_board, g1, black)
        ) t_moveList.emplace_back(Move(e1, g1, kingCastle, king));
    }
    else{
        static constexpr uint64_t longCastleSquares = (uint64_t) 0x0e00000000000000;
        static constexpr uint64_t shortCastleSquares = (uint64_t) 0x6000000000000000;
        uint64_t intersection = longCastleSquares & emptySet; 
        if(
            t_board.getLongCastle(black) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(t_board, c8, white) &&
            ! isSquareAttacked(t_board, d8, white) &&
            ! isSquareAttacked(t_board, e8, white)
        ) t_moveList.emplace_back(Move(e8, c8, queenCastle, king));

        intersection = shortCastleSquares & emptySet;
        if(
            t_board.getShortCastle(black) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(t_board, e8, white) &&
            ! isSquareAttacked(t_board, f8, white) &&
            ! isSquareAttacked(t_board, g8, white)
        ) t_moveList.emplace_back(Move(e8, g8, kingCastle, king));
    }
}

bool MoveGenerator::isSquareAttacked(const Board &t_board, int t_square, int t_attackingSide)
{
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t pawnsSet = t_board.getBitboard(pawn) & t_board.getBitboard(t_attackingSide);
    if ((m_lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;


    for (int piece = knight; piece <= king; piece ++){
        uint64_t pieceSet = t_board.getBitboard(piece) & t_board.getBitboard(t_attackingSide);
        if((m_lookup.getAttacks(piece, t_square, occupied) & pieceSet) != 0) return true;
    } 

    return false;
}
