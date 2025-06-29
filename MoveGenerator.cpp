#include "MoveGenerator.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>

void MoveGenerator::generateQuiets(uint64_t t_target, const Board &t_board, std::vector<Move> &t_moveList) const
{
    generatePawnsQuiets(t_target, t_moveList, t_board);
    generateCastle(t_moveList, t_board);
    for (int pieceType = knight; pieceType <= king; pieceType ++) generatePieceQuiets(t_target, pieceType, t_moveList, t_board);
}


void MoveGenerator::generateCaptures(uint64_t t_target, const Board &t_board, std::vector<Move> &t_moveList) const
{    
    generatePromoCaptures(t_target, t_moveList, t_board);
    generatePawnsCaptures(t_target, t_moveList, t_board);
    for (int pieceType = knight; pieceType <= king; pieceType ++) {
        generatePieceCaptures(t_target, pieceType, t_moveList, t_board);
    }
    if (t_board.getEpState()) generateEnPassants(t_target, t_moveList, t_board);
}

void MoveGenerator::evadeChecks(const Board &t_board, std::vector<Move> &t_moveList) const
{
    int kingSquare = t_board.getKingSquare(t_board.getSideToMove());
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t target = m_lookup.getAttacks(queen, kingSquare, occupied) | m_lookup.getAttacks(knight, kingSquare, occupied);

    generateCaptures(target, t_board, t_moveList);
    generateQuiets(target, t_board, t_moveList);
}

void MoveGenerator::generatePieceQuiets(uint64_t t_target, int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const
{
    uint64_t pieceSet = t_board.getBitboard(t_piece) & t_board.getBitboard(t_board.getSideToMove());
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    t_target &= ~occupied;

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = m_lookup.getAttacks(t_piece, startingSquare, occupied);

        uint64_t quietMoves = attackSet & t_target;
        if (quietMoves) do {
            int endSquare = bitScanForward(quietMoves);
            t_moveList.emplace_back(Move(startingSquare, endSquare, quiet));
        } while (quietMoves &= (quietMoves - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsQuiets(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const
{
    uint64_t emptySet = ~(t_board.getBitboard(white) | t_board.getBitboard(black));
    uint64_t pawnSet, doublePushSet, pushSet, promoSet;
    int offset;

    if(t_board.getSideToMove() == white) {
        static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
        static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4 & t_target;
        pushSet = (pawnSet << 8 & emptySet) & ~rank8 & t_target;
        promoSet = (pawnSet << 8 & emptySet) & rank8 & t_target; 
        offset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(black);
        doublePushSet = (pawnSet >> 8) & emptySet;
        doublePushSet = (doublePushSet >> 8) & emptySet & rank5 & t_target;
        pushSet = (pawnSet >> 8 & emptySet) & ~rank1 & t_target;
        promoSet = (pawnSet >> 8 & emptySet) & rank1 & t_target; 
        offset = 8;
    }

    if (pushSet) do {
        int endSq = bitScanForward(pushSet);
        int startSq = endSq + offset;
        t_moveList.emplace_back(Move(startSq, endSq, quiet));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = bitScanForward(promoSet);
        int startSq = endSq + offset;
        t_moveList.emplace_back(Move(startSq, endSq, knightPromo));
        t_moveList.emplace_back(Move(startSq, endSq, bishopPromo));
        t_moveList.emplace_back(Move(startSq, endSq, rookPromo));
        t_moveList.emplace_back(Move(startSq, endSq, queenPromo));
    } while (promoSet &= (promoSet - 1));

    if (doublePushSet) do {
        int endSq = bitScanForward(doublePushSet);
        int startSq = endSq + (2 * offset);
        t_moveList.emplace_back(Move(startSq, endSq, doublePush));
    } while (doublePushSet  &= (doublePushSet - 1));
}

void MoveGenerator::generateCastle(std::vector<Move> &t_moveList, const Board &t_board) const
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
        ) t_moveList.emplace_back(Move(e1, c1, queenCastle));
        
        intersection = shortCastleSquares & emptySet; 
        if(
            t_board.getShortCastle(white) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(t_board, e1, black) &&
            ! isSquareAttacked(t_board, f1, black) &&
            ! isSquareAttacked(t_board, g1, black)
        ) t_moveList.emplace_back(Move(e1, g1, kingCastle));
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
        ) t_moveList.emplace_back(Move(e8, c8, queenCastle));

        intersection = shortCastleSquares & emptySet;
        if(
            t_board.getShortCastle(black) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(t_board, e8, white) &&
            ! isSquareAttacked(t_board, f8, white) &&
            ! isSquareAttacked(t_board, g8, white)
        ) t_moveList.emplace_back(Move(e8, g8, kingCastle));
    }
}

void MoveGenerator::generatePieceCaptures(uint64_t t_target, int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const
{
    int stm = t_board.getSideToMove();
    uint64_t pieceSet = t_board.getBitboard(t_piece) & t_board.getBitboard(stm);
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);

    t_target &= t_board.getBitboard(1-stm);
    
    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = m_lookup.getAttacks(t_piece, startingSquare, occupied);
        uint64_t captures = attackSet & t_target;
        if (captures) do {
            int endSquare = bitScanForward(captures);
            t_moveList.emplace_back(Move(startingSquare, endSquare, capture));
        } while (captures &= (captures - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsCaptures(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const
{
    int stm = t_board.getSideToMove();
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(stm);
    uint64_t eastCaptures, westCaptures;
    int eastOffset, westOffset;

    t_target &= t_board.getBitboard(1-stm);
    
    if(stm == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastCaptures = (cpyWrapEast(pawnSet) << 8 & t_target) & ~row8;
        westCaptures = (cpyWrapWest(pawnSet) << 8 & t_target) & ~row8;
        eastOffset = -9;
        westOffset = -7;
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastCaptures = (cpyWrapEast(pawnSet) >> 8 & t_target) & ~row1;
        westCaptures = (cpyWrapWest(pawnSet) >> 8 & t_target) & ~row1;
        eastOffset = 7;
        westOffset = 9;
    }

    if (westCaptures) do {
        int endSq = bitScanForward(westCaptures);
        int startSq = endSq + westOffset;
        t_moveList.emplace_back(Move(startSq, endSq, capture));
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = bitScanForward(eastCaptures);
        int startSq = endSq + eastOffset;
        t_moveList.emplace_back(Move(startSq, endSq, capture));
    } while (eastCaptures &= (eastCaptures - 1));
}

void MoveGenerator::generatePromoCaptures(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const
{
    int stm = t_board.getSideToMove();
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(stm);
    uint64_t eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;

    t_target &= t_board.getBitboard(1-stm);

    if(stm == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastPromoCaptures = (cpyWrapEast(pawnSet) << 8 & t_target) & row8;
        westPromoCaptures = (cpyWrapWest(pawnSet) << 8 & t_target) & row8;
        eastOffset = -9;
        westOffset = -7;
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastPromoCaptures = (cpyWrapEast(pawnSet) >> 8 & t_target) & row1;
        westPromoCaptures = (cpyWrapWest(pawnSet) >> 8 & t_target) & row1;
        eastOffset = 7;
        westOffset = 9;
    }

    if (eastPromoCaptures) do {
        int endSq = bitScanForward(eastPromoCaptures);
        int startSq = endSq + eastOffset;
        t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture));
    } while (eastPromoCaptures &= (eastPromoCaptures - 1));

    if (westPromoCaptures) do {
        int endSq = bitScanForward(westPromoCaptures);
        int startSq = endSq + westOffset;
        t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture));
        t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture));
    } while (westPromoCaptures &= (westPromoCaptures - 1));
}

void MoveGenerator::generateEnPassants(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const
{
    int stm = t_board.getSideToMove();
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(stm);

    int epSquare = t_board.getEpSquare();
    uint64_t epMask = uint64_t(1) << epSquare;


    if (stm == white) {
        if(t_target & (epMask << 8)){
            if (cpyWrapEast(epMask) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare + 1, epSquare + 8, enPassant));
            if (cpyWrapWest(epMask) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare - 1, epSquare + 8, enPassant));
        }
    }
    else {
        if(t_target & (epMask >> 8)) {
            if (cpyWrapEast(epMask) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare + 1, epSquare - 8, enPassant));
            if (cpyWrapWest(epMask) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare - 1, epSquare - 8, enPassant));
        }
    }
}

bool MoveGenerator::isSquareAttacked(const Board &t_board, int t_square, int t_attackingSide) const
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