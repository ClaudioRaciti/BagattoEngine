#pragma once

#include <cstdint>
#include <vector>

#include "Move.hpp"
#include "MagicBitboards.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator() : mLookup{MagicBitboards::getInstance()} {};
    inline void generate(const Board& tBoard, std::vector<Move>& outList) const {generate(UINT64_MAX, tBoard, outList);}
    void generate (uint64_t tTarget, const Board& tBoard, std::vector<Move>& outList) const; 
    void evadeChecks(const Board&, std::vector<Move> &) const;
    bool isSquareAttacked(const Board &tBoard, int tSquare, int tAttackingSide) const;
private:
    void pieceMoves(uint64_t tTarget, int tPiece, std::vector<Move>& tList, const Board& tBoard) const;
    void pawnMoves(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    
    void castles(std::vector<Move> &tList, const Board &tBoard) const; 
    void enPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
private:
    const MagicBitboards& mLookup;
};