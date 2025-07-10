#include "MoveGenerator.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>

void MoveGenerator::generateQuiets(uint64_t tTarget, const Board &tBoard, std::vector<Move> &tList) const
{
    generatePawnsQuiets(tTarget, tList, tBoard);
    generateCastle(tList, tBoard);
    for (int pieceType = knight; pieceType <= king; pieceType ++) generatePieceQuiets(tTarget, pieceType, tList, tBoard);
}


void MoveGenerator::generateCaptures(uint64_t tTarget, const Board &tBoard, std::vector<Move> &tList) const
{    
    generatePromoCaptures(tTarget, tList, tBoard);
    generatePawnsCaptures(tTarget, tList, tBoard);
    for (int pieceType = knight; pieceType <= king; pieceType ++) {
        generatePieceCaptures(tTarget, pieceType, tList, tBoard);
    }
    if (tBoard.getEpState()) generateEnPassants(tTarget, tList, tBoard);
}

void MoveGenerator::evadeChecks(const Board &tBoard, std::vector<Move> &tList) const
{
    int kingSquare = tBoard.getKingSquare(tBoard.getSideToMove());
    uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);
    uint64_t target = mLookup.getAttacks(queen, kingSquare, occupied) | mLookup.getAttacks(knight, kingSquare, occupied);

    generateCaptures(target, tBoard, tList);
    generateQuiets(target, tBoard, tList);
}

void MoveGenerator::generatePieceQuiets(uint64_t tTarget, int tPiece, std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t pieceSet = tBoard.getBitboard(tPiece) & tBoard.getBitboard(tBoard.getSideToMove());
    uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);
    tTarget &= ~occupied;

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = mLookup.getAttacks(tPiece, startingSquare, occupied);

        uint64_t quietMoves = attackSet & tTarget;
        if (quietMoves) do {
            int endSquare = bitScanForward(quietMoves);
            tList.emplace_back(Move(startingSquare, endSquare, quiet));
        } while (quietMoves &= (quietMoves - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsQuiets(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
    uint64_t pawnSet, doublePushSet, pushSet, promoSet;
    int offset;

    if(tBoard.getSideToMove() == white) {
        static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
        static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);
        pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4 & tTarget;
        pushSet = (pawnSet << 8 & emptySet) & ~rank8 & tTarget;
        promoSet = (pawnSet << 8 & emptySet) & rank8 & tTarget; 
        offset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
        pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(black);
        doublePushSet = (pawnSet >> 8) & emptySet;
        doublePushSet = (doublePushSet >> 8) & emptySet & rank5 & tTarget;
        pushSet = (pawnSet >> 8 & emptySet) & ~rank1 & tTarget;
        promoSet = (pawnSet >> 8 & emptySet) & rank1 & tTarget; 
        offset = 8;
    }

    if (pushSet) do {
        int endSq = bitScanForward(pushSet);
        int startSq = endSq + offset;
        tList.emplace_back(Move(startSq, endSq, quiet));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = bitScanForward(promoSet);
        int startSq = endSq + offset;
        tList.emplace_back(Move(startSq, endSq, knightPromo));
        tList.emplace_back(Move(startSq, endSq, bishopPromo));
        tList.emplace_back(Move(startSq, endSq, rookPromo));
        tList.emplace_back(Move(startSq, endSq, queenPromo));
    } while (promoSet &= (promoSet - 1));

    if (doublePushSet) do {
        int endSq = bitScanForward(doublePushSet);
        int startSq = endSq + (2 * offset);
        tList.emplace_back(Move(startSq, endSq, doublePush));
    } while (doublePushSet  &= (doublePushSet - 1));
}

void MoveGenerator::generateCastle(std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t emptySet = ~(tBoard.getBitboard(black) | tBoard.getBitboard(white));

    if (tBoard.getSideToMove() == white){
        static constexpr uint64_t longCastleSquares = (uint64_t) 0x000000000000000e;
        static constexpr uint64_t shortCastleSquares = (uint64_t) 0x0000000000000060;
        uint64_t intersection = longCastleSquares & emptySet;
        if(
            tBoard.getLongCastle(white) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(tBoard, c1, black) &&
            ! isSquareAttacked(tBoard, d1, black) &&
            ! isSquareAttacked(tBoard, e1, black)
        ) tList.emplace_back(Move(e1, c1, queenCastle));
        
        intersection = shortCastleSquares & emptySet; 
        if(
            tBoard.getShortCastle(white) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(tBoard, e1, black) &&
            ! isSquareAttacked(tBoard, f1, black) &&
            ! isSquareAttacked(tBoard, g1, black)
        ) tList.emplace_back(Move(e1, g1, kingCastle));
    }
    else{
        static constexpr uint64_t longCastleSquares = (uint64_t) 0x0e00000000000000;
        static constexpr uint64_t shortCastleSquares = (uint64_t) 0x6000000000000000;
        uint64_t intersection = longCastleSquares & emptySet; 
        if(
            tBoard.getLongCastle(black) &&
            (intersection == longCastleSquares) &&
            ! isSquareAttacked(tBoard, c8, white) &&
            ! isSquareAttacked(tBoard, d8, white) &&
            ! isSquareAttacked(tBoard, e8, white)
        ) tList.emplace_back(Move(e8, c8, queenCastle));

        intersection = shortCastleSquares & emptySet;
        if(
            tBoard.getShortCastle(black) &&
            (intersection == shortCastleSquares) &&
            ! isSquareAttacked(tBoard, e8, white) &&
            ! isSquareAttacked(tBoard, f8, white) &&
            ! isSquareAttacked(tBoard, g8, white)
        ) tList.emplace_back(Move(e8, g8, kingCastle));
    }
}

void MoveGenerator::generatePieceCaptures(uint64_t tTarget, int tPiece, std::vector<Move> &tList, const Board &tBoard) const
{
    int stm = tBoard.getSideToMove();
    uint64_t pieceSet = tBoard.getBitboard(tPiece) & tBoard.getBitboard(stm);
    uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);

    tTarget &= tBoard.getBitboard(1-stm);
    
    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = mLookup.getAttacks(tPiece, startingSquare, occupied);
        uint64_t captures = attackSet & tTarget;
        if (captures) do {
            int endSquare = bitScanForward(captures);
            tList.emplace_back(Move(startingSquare, endSquare, capture));
        } while (captures &= (captures - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsCaptures(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
{
    int stm = tBoard.getSideToMove();
    uint64_t pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(stm);
    uint64_t eastCaptures, westCaptures;
    int eastOffset, westOffset;

    tTarget &= tBoard.getBitboard(1-stm);
    
    if(stm == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastCaptures = (cpyWrapEast(pawnSet) << 8 & tTarget) & ~row8;
        westCaptures = (cpyWrapWest(pawnSet) << 8 & tTarget) & ~row8;
        eastOffset = -9;
        westOffset = -7;
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastCaptures = (cpyWrapEast(pawnSet) >> 8 & tTarget) & ~row1;
        westCaptures = (cpyWrapWest(pawnSet) >> 8 & tTarget) & ~row1;
        eastOffset = 7;
        westOffset = 9;
    }

    if (westCaptures) do {
        int endSq = bitScanForward(westCaptures);
        int startSq = endSq + westOffset;
        tList.emplace_back(Move(startSq, endSq, capture));
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = bitScanForward(eastCaptures);
        int startSq = endSq + eastOffset;
        tList.emplace_back(Move(startSq, endSq, capture));
    } while (eastCaptures &= (eastCaptures - 1));
}

void MoveGenerator::generatePromoCaptures(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
{
    int stm = tBoard.getSideToMove();
    uint64_t pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(stm);
    uint64_t eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;

    tTarget &= tBoard.getBitboard(1-stm);

    if(stm == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastPromoCaptures = (cpyWrapEast(pawnSet) << 8 & tTarget) & row8;
        westPromoCaptures = (cpyWrapWest(pawnSet) << 8 & tTarget) & row8;
        eastOffset = -9;
        westOffset = -7;
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastPromoCaptures = (cpyWrapEast(pawnSet) >> 8 & tTarget) & row1;
        westPromoCaptures = (cpyWrapWest(pawnSet) >> 8 & tTarget) & row1;
        eastOffset = 7;
        westOffset = 9;
    }

    if (eastPromoCaptures) do {
        int endSq = bitScanForward(eastPromoCaptures);
        int startSq = endSq + eastOffset;
        tList.emplace_back(Move(startSq, endSq, knightPromoCapture));
        tList.emplace_back(Move(startSq, endSq, bishopPromoCapture));
        tList.emplace_back(Move(startSq, endSq, rookPromoCapture));
        tList.emplace_back(Move(startSq, endSq, queenPromoCapture));
    } while (eastPromoCaptures &= (eastPromoCaptures - 1));

    if (westPromoCaptures) do {
        int endSq = bitScanForward(westPromoCaptures);
        int startSq = endSq + westOffset;
        tList.emplace_back(Move(startSq, endSq, knightPromoCapture));
        tList.emplace_back(Move(startSq, endSq, bishopPromoCapture));
        tList.emplace_back(Move(startSq, endSq, rookPromoCapture));
        tList.emplace_back(Move(startSq, endSq, queenPromoCapture));
    } while (westPromoCaptures &= (westPromoCaptures - 1));
}

void MoveGenerator::generateEnPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
{
    int stm = tBoard.getSideToMove();
    uint64_t pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(stm);

    int epSquare = tBoard.getEpSquare();
    uint64_t epMask = uint64_t(1) << epSquare;


    if (stm == white) {
        if(tTarget & (epMask << 8)){
            if (cpyWrapEast(epMask) & pawnSet) 
                tList.emplace_back(Move(epSquare + 1, epSquare + 8, enPassant));
            if (cpyWrapWest(epMask) & pawnSet) 
                tList.emplace_back(Move(epSquare - 1, epSquare + 8, enPassant));
        }
    }
    else {
        if(tTarget & (epMask >> 8)) {
            if (cpyWrapEast(epMask) & pawnSet) 
                tList.emplace_back(Move(epSquare + 1, epSquare - 8, enPassant));
            if (cpyWrapWest(epMask) & pawnSet) 
                tList.emplace_back(Move(epSquare - 1, epSquare - 8, enPassant));
        }
    }
}

bool MoveGenerator::isSquareAttacked(const Board &tBoard, int tSquare, int tAttackingSide) const
{
    uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);
    uint64_t pawnsSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(tAttackingSide);
    if ((mLookup.pawnAttacks(tSquare, 1-tAttackingSide) & pawnsSet) != 0) return true;


    for (int piece = knight; piece <= king; piece ++){
        uint64_t pieceSet = tBoard.getBitboard(piece) & tBoard.getBitboard(tAttackingSide);
        if((mLookup.getAttacks(piece, tSquare, occupied) & pieceSet) != 0) return true;
    } 

    return false;
}