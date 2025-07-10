#pragma once

#include <vector>

#include "Move.hpp"
#include "MagicBitboards.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator() : mLookup{MagicBitboards::getInstance()} {};
    inline void generateMoves (const Board& tBoard, std::vector<Move>& tList) const {generateCaptures(tBoard, tList); generateQuiets(tBoard, tList);}
    inline void generateQuiets(const Board& tBoard, std::vector<Move>& tList) const {generateQuiets(UINT64_MAX, tBoard, tList);}
    inline void generateCaptures(const Board& tBoard, std::vector<Move>& tList) const {generateCaptures(UINT64_MAX, tBoard, tList);}
    void evadeChecks(const Board&, std::vector<Move> &) const;
    bool isSquareAttacked(const Board &tBoard, int tSquare, int tAttackingSide) const;
private:
    void generateQuiets(uint64_t tTarget, const Board& tBoard, std::vector<Move>& tList) const;
    void generateCaptures(uint64_t tTarget, const Board& tBoard, std::vector<Move>& tList) const;

    void generatePieceQuiets(uint64_t tTarget, int tPiece, std::vector<Move> &tList, const Board &tBoard) const;
    void generatePawnsQuiets(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    void generateCastle(std::vector<Move> &tList, const Board &tBoard) const; 

    void generatePieceCaptures(uint64_t tTarget, int tPiece, std::vector<Move> &tList, const Board &tBoard) const;
    void generatePawnsCaptures(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    void generatePromoCaptures(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    void generateEnPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
private:
    const MagicBitboards& mLookup;
};