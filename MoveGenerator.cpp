#include "MoveGenerator.hpp"
#include "Board.hpp"
#include "Move.hpp"
#include "notation.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>
#include <vector>

void MoveGenerator::generate(uint64_t tTarget, const Board& tBoard, std::vector<Move>& tList) const{
    for (int piece = knight; piece <= king; piece ++) pieceMoves(tTarget, piece, tList, tBoard);
    pawnMoves(tTarget, tList, tBoard);
    
    if(tBoard.getEpState() == true) enPassants(tTarget, tList, tBoard);

    castles(tList, tBoard);
}

void MoveGenerator::pieceMoves(uint64_t tTarget, int tPiece, std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t pieceSet = tBoard.getBitboard(tPiece) & tBoard.getBitboard(tBoard.getSideToMove());
    uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);
    uint64_t enemySet = tBoard.getBitboard(1 - tBoard.getSideToMove());

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = mLookup.getAttacks(tPiece, startingSquare, occupied);

        uint64_t quietMoves = attackSet & tTarget & ~occupied;
        if (quietMoves) do {
            int endSquare = bitScanForward(quietMoves);
            tList.emplace_back(Move(startingSquare, endSquare, quiet));
        } while (quietMoves &= (quietMoves - 1));

        uint64_t captures = attackSet & tTarget & enemySet;
        if (captures) do {
            int endSquare = bitScanForward(captures);
            tList.emplace_back(Move(startingSquare, endSquare, capture));
        } while (captures &= (captures - 1));
    } while (pieceSet &= (pieceSet - 1));
}



void MoveGenerator::pawnMoves(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t doublePushSet, pushSet, promoSet, eastCaptures, westCaptures, eastPromoCaptures, westPromoCaptures;
    int pushOffset, eastOffset, westOffset;


    if(tBoard.getSideToMove() == white) {
        static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
        static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);

        const uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
        const uint64_t enemySet = tBoard.getBitboard(black);
        const uint64_t pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(white);

        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4 & tTarget;
        pushSet  = pawnSet << 8 & ~rank8 & emptySet & tTarget;
        promoSet = pawnSet << 8 &  rank8 & emptySet & tTarget;
        eastCaptures      = cpyWrapEast(pawnSet) << 8 & ~rank8 & enemySet & tTarget;
        eastPromoCaptures = cpyWrapEast(pawnSet) << 8 &  rank8 & enemySet & tTarget;
        westCaptures      = cpyWrapWest(pawnSet) << 8 & ~rank8 & enemySet & tTarget;
        westPromoCaptures = cpyWrapWest(pawnSet) << 8 &  rank8 & enemySet & tTarget;

        eastOffset = -9;
        westOffset = -7;
        pushOffset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);

        const uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
        const uint64_t enemySet = tBoard.getBitboard(white);
        const uint64_t pawnSet = tBoard.getBitboard(pawn) & tBoard.getBitboard(black);

        doublePushSet = ((pawnSet >> 8) & emptySet) >> 8 & emptySet & rank5 & tTarget;
        pushSet  = pawnSet >> 8 & ~rank1 & emptySet & tTarget;
        promoSet = pawnSet >> 8 &  rank1 & emptySet & tTarget;
        eastCaptures      = cpyWrapEast(pawnSet) >> 8 & ~rank1 & enemySet & tTarget;
        eastPromoCaptures = cpyWrapEast(pawnSet) >> 8 &  rank1 & enemySet & tTarget;
        westCaptures      = cpyWrapWest(pawnSet) >> 8 & ~rank1 & enemySet & tTarget;
        westPromoCaptures = cpyWrapWest(pawnSet) >> 8 &  rank1 & enemySet & tTarget;

        eastOffset = 7;
        westOffset = 9;
        pushOffset = 8;
    }

    if (doublePushSet) do {
        int endSq = bitScanForward(doublePushSet);
        int startSq = endSq + (2 * pushOffset);
        tList.emplace_back(Move(startSq, endSq, doublePush));
    } while (doublePushSet  &= (doublePushSet - 1));

    if (pushSet) do {
        int endSq = bitScanForward(pushSet);
        int startSq = endSq + pushOffset;
        tList.emplace_back(Move(startSq, endSq, quiet));
    } while (pushSet &= (pushSet - 1));

    if (promoSet) do {
        int endSq = bitScanForward(promoSet);
        int startSq = endSq + pushOffset;
        tList.emplace_back(Move(startSq, endSq, knightPromo));
        tList.emplace_back(Move(startSq, endSq, bishopPromo));
        tList.emplace_back(Move(startSq, endSq, rookPromo));
        tList.emplace_back(Move(startSq, endSq, queenPromo));
    } while (promoSet &= (promoSet - 1));


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

void MoveGenerator::castles(std::vector<Move> &tList, const Board &tBoard) const
{
    uint64_t emptySet = ~(tBoard.getBitboard(black) | tBoard.getBitboard(white));

    if (tBoard.getSideToMove() == white){
        static constexpr uint64_t longCastleSquares = uint64_t(0x000000000000000e);
        static constexpr uint64_t shortCastleSquares = uint64_t(0x0000000000000060);
        uint64_t intersection = longCastleSquares & emptySet;
        if(
            tBoard.getLongCastle(white) &&
            (intersection == longCastleSquares) &&
            ! isAttacked(tBoard, c1, black) &&
            ! isAttacked(tBoard, d1, black) &&
            ! isAttacked(tBoard, e1, black)
        ) tList.emplace_back(Move(e1, c1, queenCastle));
        
        intersection = shortCastleSquares & emptySet; 
        if(
            tBoard.getShortCastle(white) &&
            (intersection == shortCastleSquares) &&
            ! isAttacked(tBoard, e1, black) &&
            ! isAttacked(tBoard, f1, black) &&
            ! isAttacked(tBoard, g1, black)
        ) tList.emplace_back(Move(e1, g1, kingCastle));
    }
    else{
        static constexpr uint64_t longCastleSquares = uint64_t(0x0e00000000000000);
        static constexpr uint64_t shortCastleSquares = uint64_t(0x6000000000000000);
        uint64_t intersection = longCastleSquares & emptySet; 
        if(
            tBoard.getLongCastle(black) &&
            (intersection == longCastleSquares) &&
            ! isAttacked(tBoard, c8, white) &&
            ! isAttacked(tBoard, d8, white) &&
            ! isAttacked(tBoard, e8, white)
        ) tList.emplace_back(Move(e8, c8, queenCastle));

        intersection = shortCastleSquares & emptySet;
        if(
            tBoard.getShortCastle(black) &&
            (intersection == shortCastleSquares) &&
            ! isAttacked(tBoard, e8, white) &&
            ! isAttacked(tBoard, f8, white) &&
            ! isAttacked(tBoard, g8, white)
        ) tList.emplace_back(Move(e8, g8, kingCastle));
    }
}

void MoveGenerator::enPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const
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

bool MoveGenerator::isAttacked(const Board &tBoard, int tSquare, int tAttackingSide) const
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

bool MoveGenerator::validate(const Board& tBoard,const Move tMove) const {
    int moved = tBoard.searchPiece(tMove.from());

    if (moved == pawn){
        // Insures that ep is available and the right file is used
        if (tMove.isEnPassant())
            return tBoard.getEpState() && (tBoard.getEpSquare()%8 == tMove.to()%8);

        int stm = tBoard.getSideToMove();
        uint64_t moveMask = uint64_t(1) << tMove.to();

        // Insures pawns capture according to their attack pattern and move_to square is occupied
        if (tMove.isCapture())
            return (mLookup.pawnAttacks(tMove.from(), stm) & moveMask) && tBoard.searchPiece(tMove.to());
    
        // Insures pawns don't push trough existing pieces
        uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
        uint64_t moveSet  = uint64_t(1) << tMove.from();
        moveSet  = (stm == white ? moveSet << 8 : moveSet >> 8) & emptySet;

        if (tMove.isDoublePush()) // move_to square correctness doesn't need to be checked
            return (stm == white ? moveSet << 8 : moveSet >> 8) & emptySet;
        
        return moveSet & moveMask; // Checks move_to square correnctess
    }

    if (moved != 0){
        if (tMove.isPromo() || tMove.isEnPassant() || tMove.isDoublePush())
            return false; // These flags are pawn exclusive
        
        int stm = tBoard.getSideToMove();
        uint64_t occupied   = tBoard.getBitboard(white) | tBoard.getBitboard(black);

        auto isKingSafe = [&] (uint64_t mask) {
            do {
                int square = bitScanForward(mask);
                if (isAttacked(tBoard, square, stm))
                    return false;
            } while (mask &= (mask - 1));
            return true;
        };

        if (tMove.isCastle()) {
            static constexpr std::array<uint64_t, 4> castleSets = {
                uint64_t(0x0000000000000060), uint64_t(0x000000000000000e),
                uint64_t(0x6000000000000000), uint64_t(0x0e00000000000000)
            };
            uint64_t castleMask = castleSets[(tMove.flag() - kingCastle) + (2 * stm)];
            return (castleMask & ~occupied) && isKingSafe(castleMask);
        }
        
        // Insures pieces move according to their attack patterns
        uint64_t attackSet = mLookup.getAttacks(moved, tMove.from(), occupied);
        uint64_t moveMask  = uint64_t(1) << tMove.to();
        bool captureFound  = tBoard.searchPiece(tMove.to());

        // Checks that move_to square is occupied iff move is capture
        return (attackSet & moveMask) && (tMove.isCapture() == captureFound);
    } 

    return false;
}
