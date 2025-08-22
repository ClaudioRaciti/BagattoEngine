#pragma once

#include <cstdint>
#include <vector>

#include "Move.hpp"
#include "MagicBitboards.hpp"
#include "Board.hpp"
#include "notation.hpp"

class MoveGenerator{
public:
    MoveGenerator() : mLookup{MagicBitboards::getInstance()} {};
    inline void all(const Board& tBoard, std::vector<Move>& outList) const {
        generate(UINT64_MAX, tBoard, outList);
    }
    inline void captures(const Board& tBoard, std::vector<Move>& outList) const {
        uint64_t enemySet = tBoard.getBitboard(1 - tBoard.getSideToMove());
        generate(enemySet, tBoard, outList);
    }
    inline void quiets(const Board& tBoard, std::vector<Move>& outList) const {
        uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
        generate(emptySet, tBoard, outList);
    }
    void generate (uint64_t tTarget, const Board& tBoard, std::vector<Move>& outList) const; 
    void evadeChecks(const Board&, std::vector<Move> &) const;
    bool isAttacked(const Board &tBoard, int tSquare, int tAttackingSide) const;
    bool isPseudoLegal(const Board& tBoard,const Move tMove) const;
private:
    void pieceMoves(uint64_t tTarget, int tPiece, std::vector<Move>& tList, const Board& tBoard) const;
    void pawnMoves(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    
    void castles(std::vector<Move> &tList, const Board &tBoard) const; 
    void enPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
private:
    const MagicBitboards& mLookup;
};