#include "MoveGenerator.hpp"
#include "utils.hpp"
#include <iostream>

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

void MoveGenerator::evadeCheck(const Board &t_board, std::vector<Move> &t_moveList)
{
    generatePieceQuiets(king, t_moveList, t_board);
    
    int sideToMove = t_board.getSideToMove();
    int kingSquare = t_board.getKingSquare(sideToMove);
    uint64_t checkers = getAttacksTo(t_board, kingSquare, 1 - sideToMove);
    if(popCount(checkers) == 1){
        int checkingSquare = bitScanForward(checkers);
        int checkingPiece;
        for (checkingPiece = pawn; checkingPiece < king; checkingPiece ++)
            if (t_board.getBitboard(checkingPiece) & checkers) break;
        
        captureAttacker(checkingPiece, checkingSquare, t_moveList, t_board);
        //find out wich piece is the attacker
        //if attacker is sliding piece: block
        if (checkingPiece == bishop || checkingPiece == queen){
            uint64_t blockableSquares;
            if (m_lookup.rayAttacks(kingSquare, soWe) & checkers) {
                blockableSquares = m_lookup.bishopAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, soWe) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, noWe) & checkers) {
                blockableSquares = m_lookup.bishopAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, noWe) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, soEa) & checkers) {
                blockableSquares = m_lookup.bishopAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, soEa) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, noEa) & checkers) {
                blockableSquares = m_lookup.bishopAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, noEa) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
        }
        if (checkingPiece == rook || checkingPiece == queen){
            uint64_t blockableSquares;
            if (m_lookup.rayAttacks(kingSquare, sout) & checkers) {
                blockableSquares = m_lookup.rookAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, sout) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, nort) & checkers) {
                blockableSquares = m_lookup.rookAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, nort) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, east) & checkers) {
                blockableSquares = m_lookup.rookAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, east) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
            else if (m_lookup.rayAttacks(kingSquare, west) & checkers) {
                blockableSquares = m_lookup.rookAttacks(checkers, kingSquare) & m_lookup.rayAttacks(kingSquare, west) & ~checkers;
                blockAttacker(blockableSquares, t_moveList, t_board);
            }
        }
    }
}

void MoveGenerator::generatePieceQuiets(int t_piece, std::vector<Move> &t_moveList, const Board &t_board)
{
    uint64_t pieceSet = t_board.getBitboard(t_piece) & t_board.getBitboard(t_board.getSideToMove());
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t emptySet = ~occupied;

    if (pieceSet) do {
        int startingSquare = bitScanForward(pieceSet);
        uint64_t attackSet = m_lookup.getAttacks(t_piece, startingSquare, occupied);

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
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4;
        pushSet = (pawnSet << 8 & emptySet) & ~rank8;
        promoSet = (pawnSet << 8 & emptySet) & rank8; 
        offset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(black);
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
            uint64_t captureMask = uint64_t(1) << endSquare;
            for (int captured  = pawn; captured < king; captured ++) 
                if (captureMask & t_board.getBitboard(captured)) {
                    t_moveList.emplace_back(Move(startingSquare, endSquare, capture, t_piece, captured));
                    break;
                }
        } while (captures &= (captures - 1));
    } while (pieceSet &= (pieceSet - 1));
}

void MoveGenerator::generatePawnsCaptures(std::vector<Move> &t_moveList, const Board &t_board)
{
    int sideToMove = t_board.getSideToMove();
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(sideToMove);
    uint64_t enemyPieces = t_board.getBitboard(1 - sideToMove);
    uint64_t eastCaptures, westCaptures, eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;
    
    if(sideToMove == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastCaptures = (cpyWrapEast(pawnSet) << 8 & enemyPieces) & ~row8;
        westCaptures = (cpyWrapWest(pawnSet) << 8 & enemyPieces) & ~row8;
        eastPromoCaptures = (cpyWrapEast(pawnSet) << 8 & enemyPieces) & row8;
        westPromoCaptures = (cpyWrapWest(pawnSet) << 8 & enemyPieces) & row8;
        eastOffset = -9;
        westOffset = -7;

        if(t_board.getEpState() == true){
            int epSquare = t_board.getEpSquare();
            if (cpyWrapEast(uint64_t(1) << epSquare) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare + 1, epSquare + 8, enPassant, pawn, pawn));
            if (cpyWrapWest(uint64_t(1) << epSquare) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare - 1, epSquare + 8, enPassant, pawn, pawn));
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
            if (cpyWrapEast(uint64_t(1) << epSquare) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare + 1, epSquare - 8, enPassant, pawn, pawn));
            if (cpyWrapWest(uint64_t(1) << epSquare) & pawnSet) 
                t_moveList.emplace_back(Move(epSquare - 1, epSquare - 8, enPassant, pawn, pawn));
        }
    }

    if (westCaptures) do {
        int endSq = bitScanForward(westCaptures);
        uint64_t captureMask = uint64_t(1) << endSq;
        int startSq = endSq + westOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (captureMask & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, capture, pawn, captured));
                break;
            }
    } while (westCaptures &= (westCaptures - 1));


    if (eastCaptures) do {
        int endSq = bitScanForward(eastCaptures);
        uint64_t captureMask = uint64_t(1) << endSq;
        int startSq = endSq + eastOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (captureMask & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, capture, pawn, captured));
                break;
            }
    } while (eastCaptures &= (eastCaptures - 1));


    if (eastPromoCaptures) do {
        int endSq = bitScanForward(eastPromoCaptures);
        uint64_t captureMask = uint64_t(1) << endSq;
        int startSq = endSq + eastOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (captureMask & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture, pawn, captured));
                break;
            }
    } while (eastPromoCaptures &= (eastPromoCaptures - 1));

    if (westPromoCaptures) do {
        int endSq = bitScanForward(westPromoCaptures);
        uint64_t captureMask = uint64_t(1) << endSq;
        int startSq = endSq + westOffset;
        for (int captured  = pawn; captured < king; captured ++) 
            if (captureMask & t_board.getBitboard(captured)) {
                t_moveList.emplace_back(Move(startSq, endSq, knightPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, bishopPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, rookPromoCapture, pawn, captured));
                t_moveList.emplace_back(Move(startSq, endSq, queenPromoCapture, pawn, captured));
                break;
            }
    } while (westPromoCaptures &= (westPromoCaptures - 1));
}

void MoveGenerator::captureAttacker(int t_attacker, int t_attackerSq, std::vector<Move> &t_moveList, const Board &t_board)
{
    uint64_t attackerMask = uint64_t(1) << t_attackerSq;
    int sideToMove = t_board.getSideToMove();
    //insert captures to the attacker
    //pawn logic
    uint64_t pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(sideToMove);
    uint64_t eastCaptures, westCaptures, eastPromoCaptures, westPromoCaptures;
    int eastOffset, westOffset;
    
    if(sideToMove == white) {
        static constexpr uint64_t row8 = uint64_t(0xff00000000000000);
        eastCaptures = (cpyWrapEast(pawnSet) << 8 & attackerMask) & ~row8;
        westCaptures = (cpyWrapWest(pawnSet) << 8 & attackerMask) & ~row8;
        eastPromoCaptures = (cpyWrapEast(pawnSet) << 8 & attackerMask) & row8;
        westPromoCaptures = (cpyWrapWest(pawnSet) << 8 & attackerMask) & row8;
        eastOffset = -9;
        westOffset = -7;

        if((t_board.getEpState() == true) && (t_board.getEpSquare() == t_attackerSq)){
            if (cpyWrapEast(attackerMask) & pawnSet) 
                t_moveList.emplace_back(Move(t_attackerSq + 1, t_attackerSq + 8, enPassant, pawn, pawn));
            if (cpyWrapWest(attackerMask) & pawnSet) 
                t_moveList.emplace_back(Move(t_attackerSq - 1, t_attackerSq + 8, enPassant, pawn, pawn));
        }
    }
    else {
        static constexpr uint64_t row1 = uint64_t(0x00000000000000ff);
        eastCaptures = (cpyWrapEast(pawnSet) >> 8 & attackerMask) & ~row1;
        westCaptures = (cpyWrapWest(pawnSet) >> 8 & attackerMask) & ~row1;
        eastPromoCaptures = (cpyWrapEast(pawnSet) >> 8 & attackerMask) & row1;
        westPromoCaptures = (cpyWrapWest(pawnSet) >> 8 & attackerMask) & row1;
        eastOffset = 7;
        westOffset = 9;

        if((t_board.getEpState() == true) && (t_board.getEpSquare() == t_attackerSq)){
            if (cpyWrapEast(attackerMask) & pawnSet) 
                t_moveList.emplace_back(Move(t_attackerSq + 1, t_attackerSq - 8, enPassant, pawn, pawn));
            if (cpyWrapWest(attackerMask) & pawnSet) 
                t_moveList.emplace_back(Move(t_attackerSq - 1, t_attackerSq - 8, enPassant, pawn, pawn));
        }
    }

    if (westCaptures) t_moveList.emplace_back(Move(t_attackerSq + westOffset, t_attackerSq, capture, pawn, t_attacker));
    if (eastCaptures) t_moveList.emplace_back(Move(t_attackerSq + eastOffset, t_attackerSq, capture, pawn, t_attacker));
    if (westPromoCaptures){
        t_moveList.emplace_back(Move(t_attackerSq + westOffset, t_attackerSq, knightPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + westOffset, t_attackerSq, bishopPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + westOffset, t_attackerSq, rookPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + westOffset, t_attackerSq, queenPromoCapture, pawn, t_attacker));
    }
    if (eastPromoCaptures){
        t_moveList.emplace_back(Move(t_attackerSq + eastOffset, t_attackerSq, knightPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + eastOffset, t_attackerSq, bishopPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + eastOffset, t_attackerSq, rookPromoCapture, pawn, t_attacker));
        t_moveList.emplace_back(Move(t_attackerSq + eastOffset, t_attackerSq, queenPromoCapture, pawn, t_attacker));
    }

    //pieces logic
    for (int capturingPiece = knight; capturingPiece <= king; capturingPiece ++){
        uint64_t pieceSet = t_board.getBitboard(capturingPiece) & t_board.getBitboard(sideToMove);
        uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
        
        if (pieceSet) do {
            int startingSquare = bitScanForward(pieceSet);
            uint64_t attackSet = m_lookup.getAttacks(capturingPiece, startingSquare, occupied);
            uint64_t captureSet = attackSet & attackerMask;
            if (captureSet) t_moveList.emplace_back(Move(startingSquare, t_attackerSq, capture, capturingPiece, t_attacker));
        } while (pieceSet &= (pieceSet - 1));
    }
}

void MoveGenerator::blockAttacker(uint64_t t_blockableSquares, std::vector<Move> &t_moveList, const Board &t_board)
{
    
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t emptySet = ~occupied;
    uint64_t pawnSet, doublePushSet, pushSet, promoSet;
    int offset;

    if(t_board.getSideToMove() == white) {
        static constexpr uint64_t rank4 = uint64_t(0x00000000ff000000);
        static constexpr uint64_t rank8 = uint64_t(0xff00000000000000);
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(white);
        doublePushSet = (pawnSet << 8) & emptySet;
        doublePushSet = (doublePushSet << 8) & emptySet & rank4 & t_blockableSquares;
        pushSet = (pawnSet << 8) & emptySet & ~rank8 & t_blockableSquares;
        promoSet = (pawnSet << 8) & emptySet & rank8 & t_blockableSquares; 
        offset = -8;
    }
    else {
        static constexpr uint64_t rank5 = uint64_t(0x000000ff00000000);
        static constexpr uint64_t rank1 = uint64_t(0x00000000000000ff);
        pawnSet = t_board.getBitboard(pawn) & t_board.getBitboard(black);
        doublePushSet = (pawnSet >> 8) & emptySet;
        doublePushSet = (doublePushSet >> 8) & emptySet & rank5 & t_blockableSquares;
        pushSet = (pawnSet >> 8) & emptySet & ~rank1 & t_blockableSquares;
        promoSet = (pawnSet >> 8) & emptySet & rank1 & t_blockableSquares; 
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


    for(int piece = knight; piece < king; piece ++){
        uint64_t pieceSet = t_board.getBitboard(piece) & t_board.getBitboard(t_board.getSideToMove());
        if (pieceSet) do {
            int startingSquare = bitScanForward(pieceSet);
            uint64_t attackSet = m_lookup.getAttacks(piece, startingSquare, occupied);

            uint64_t blocks = attackSet & t_blockableSquares;
            if (blocks) do {
                int endSquare = bitScanForward(blocks);
                t_moveList.emplace_back(Move(startingSquare, endSquare, quiet, piece));
            } while (blocks &= (blocks - 1));
        } while (pieceSet &= (pieceSet - 1));
    }
}

bool MoveGenerator::isCheck(const Board &t_board)
{
    int sideToMove = t_board.getSideToMove();
    int kingSquare = t_board.getKingSquare(sideToMove);
    return isSquareAttacked(t_board, kingSquare, 1 - sideToMove);
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

uint64_t MoveGenerator::getAttacksTo(const Board &t_board, int t_square, int t_attackingSide)
{
    uint64_t occupied = t_board.getBitboard(white) | t_board.getBitboard(black);
    uint64_t enemyPawns = t_board.getBitboard(pawn) & t_board.getBitboard(t_attackingSide);
    uint64_t enemyKnights = t_board.getBitboard(knight) & t_board.getBitboard(t_attackingSide);
    uint64_t enemyBihsops = t_board.getBitboard(bishop) & t_board.getBitboard(t_attackingSide);
    uint64_t enemyRooks = t_board.getBitboard(rook) & t_board.getBitboard(t_attackingSide);
    uint64_t enemyQueens = t_board.getBitboard(queen) & t_board.getBitboard(t_attackingSide);
    uint64_t enemyKing = t_board.getBitboard(king) & t_board.getBitboard(t_attackingSide);

    return (m_lookup.pawnAttacks(t_square, 1-t_attackingSide) & enemyPawns)
        | (m_lookup.getAttacks(knight, t_square, occupied) & enemyKnights)
        | (m_lookup.getAttacks(bishop, t_square, occupied) & (enemyBihsops | enemyQueens))
        | (m_lookup.getAttacks(rook, t_square, occupied) & (enemyRooks | enemyQueens))
        | (m_lookup.getAttacks(king, t_square, occupied) & enemyKing);
}
