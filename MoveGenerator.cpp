#include "MoveGenerator.hpp"
#include "utils.hpp"

MoveGenerator::MoveGenerator() : m_lookup{LookupTables::getInstance()}{
}

std::vector<Move> MoveGenerator::generateMoves(const Board &t_board)
{
    std::vector<Move> moveList;
    moveList.reserve(256); // over the maximum number of moves possible for any legal position
    generateQuiets(t_board, moveList);
    generateCaptures(t_board, moveList);
    return moveList;
}

std::vector<Move> MoveGenerator::generateQuiets(const Board &t_board)
{
    std::vector<Move> moveList;
    moveList.reserve(256); // over the maximum number of moves possible for any legal position
    generateQuiets(t_board, moveList);
    return moveList;
}

void MoveGenerator::generateQuiets(const Board &t_board, std::vector<Move> &t_moveList)
{
    generatePawnsQuiets(t_moveList, t_board);
    generateCatstle(t_moveList, t_board);
    for (int pieceType = knight; pieceType <= king; pieceType ++) generatePieceQuiets(pieceType, t_moveList, t_board);
}

std::vector<Move> MoveGenerator::generateCaptures(const Board &t_board)
{    
    std::vector<Move> moveList;
    moveList.reserve(256); // over the maximum number of moves possible for any legal position
    generateCaptures(t_board, moveList);
    return moveList;
}

void MoveGenerator::generateCaptures(const Board &t_board, std::vector<Move> &t_moveList)
{
    generatePawnsCaptures(t_moveList, t_board);
    for (int pieceType = knight; pieceType <= king; pieceType ++) generatePieceCaptures(pieceType, t_moveList, t_board);  
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
    uint64_t emptySet = ~(t_board.getBitboard(white) | t_board.getBitboard(black));
    uint64_t pawnSet, doublePushSet, pushSet, promoSet;
    int offset;

    if(t_board.getSideToMove() == white) {
        static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
        static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);
        pawnSet = t_board.getBitboard(pawn) | t_board.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4;
        pushSet = (pawnSet << 8 & emptySet) & ~rank8;
        promoSet = (pawnSet << 8 & emptySet) & rank8; 
        offset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
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

void MoveGenerator::generatePieceCaptures(int t_piece, std::vector<Move> &t_moveList, const Board &t_board)
{
    int sideToMove = t_board.getSideToMove();
    uint64_t pieceSet = t_board.getBitboard(t_piece) & t_board.getBitboard(sideToMove);
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t enemyPieces = t_board.getBitboard(1 - sideToMove);
    
    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = m_lookup.getAttacks(t_piece, startingSquare, occupied);

        //serialize all captures moves
        uint64_t captures = attackSet & enemyPieces;
        if (captures) do {
            int endSquare = bitScanForward(captures);
            for (int captured  = pawn; captured < king; captured ++) 
                if (uint64_t(1 << endSquare) & t_board.getBitboard(captured)) {
                    t_moveList.emplace_back(Move(startingSquare, endSquare, capture, t_piece, captured));
                    break;
                }
        } while (captures &= (captures -1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsCaptures(std::vector<Move> &t_moveList, const Board &t_board)
{
    int sideToMove = t_board.getSideToMove();
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(sideToMove);
    uint64_t enemyPieces = t_board.getBitboard(1 - sideToMove);
    uint64_t eastCaptures, westCaptures, eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;
    
    if(t_board.getSideToMove() == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastCaptures = (cpyWrapEast(pawnSet) << 8 & enemyPieces) & ~row8;
        westCaptures = (cpyWrapWest(pawnSet) << 8 & enemyPieces) & ~row8;
        eastPromoCaptures = (cpyWrapEast(pawnSet) << 8 & enemyPieces) & row8;
        westPromoCaptures = (cpyWrapWest(pawnSet) << 8 & enemyPieces) & row8;
        eastOffset = -9;
        westOffset = -7;

        if(t_board.getEpState() == true){
            int epSquare = t_board.getEpSquare();
            if (cpyWrapEast(uint64_t(1) << epSquare) & pawnSet) t_moveList.emplace_back(Move(epSquare + 1, epSquare + 8, enPassant, pawn, pawn));
            if (cpyWrapWest(uint64_t(1) << epSquare) & pawnSet) t_moveList.emplace_back(Move(epSquare - 1, epSquare + 8, enPassant, pawn, pawn));
        }
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastCaptures = (cpyWrapEast(pawnSet) >> 8 & enemyPieces) & ~row1;
        westCaptures = (cpyWrapWest(pawnSet) >> 8 & enemyPieces) & ~row1;
        eastPromoCaptures = (cpyWrapEast(pawnSet) >> 8 & enemyPieces) & row1;
        westPromoCaptures = (cpyWrapWest(pawnSet) >> 8 & enemyPieces) & row1;
        eastOffset = 7;
        westOffset = 9;


        if(t_board.getEpState() == true){
            int epSquare = t_board.getEpSquare();
            if (cpyWrapEast(uint64_t(1) << epSquare) & pawnSet) t_moveList.emplace_back(Move(epSquare + 1, epSquare - 8, enPassant, pawn, pawn));
            if (cpyWrapWest(uint64_t(1) << epSquare) & pawnSet) t_moveList.emplace_back(Move(epSquare - 1, epSquare - 8, enPassant, pawn, pawn));
        }
    }

    if (westCaptures) do {
        int endSq = bitScanForward(westCaptures);
        int startSq = endSq + westOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (uint64_t(1 << endSq) & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, capture, pawn, captured));
                break;
            }
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = bitScanForward(eastCaptures);
        int startSq = endSq + eastOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (uint64_t(1 << endSq) & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, capture, pawn, captured));
                break;
            }
    } while (eastCaptures &= (eastCaptures - 1));


    if (eastPromoCaptures) do {
        int endSq = bitScanForward(eastPromoCaptures);
        int startSq = endSq + eastOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (uint64_t(1 << endSq) & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture, pawn, captured));
                break;
            }
    } while (eastPromoCaptures &= (eastPromoCaptures - 1));

    if (westPromoCaptures) do {
        int endSq = bitScanForward(westPromoCaptures);
        int startSq = endSq + westOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (uint64_t(1 << endSq) & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture, pawn, captured));
                break;
            }
    } while (westPromoCaptures &= (westPromoCaptures - 1));
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
