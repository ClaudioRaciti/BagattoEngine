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

    /**
     * @brief Generates all pseudo-legal moves
     *
     * @param tBoard The position from wich moves are computed
     * @param outList Reference to a vector to wich the moves will be appended
     * @return Nothing
     */
    inline void all(const Board& tBoard, std::vector<Move>& outList) const {
        generate(UINT64_MAX, tBoard, outList);
    }

    /**
     * @brief Generates only pseudo-legal captures
     * 
     * @param tBoard The position from wich moves are computed
     * @param outList Reference to a vector to wich the moves will be appended
     * @return Nothing
     */
    inline void captures(const Board& tBoard, std::vector<Move>& outList) const {
        uint64_t enemySet = tBoard.getBitboard(1 - tBoard.getSideToMove());
        generate(enemySet, tBoard, outList);
    }

    /**
     * @brief Generates only pseudo-legal quiet moves
     * 
     * @param tBoard The position from wich moves are computed
     * @param outList Reference to a vector to wich the moves will be appended
     * @return Nothing
     */
    inline void quiets(const Board& tBoard, std::vector<Move>& outList) const {
        uint64_t emptySet = ~(tBoard.getBitboard(white) | tBoard.getBitboard(black));
        generate(emptySet, tBoard, outList);
    }

    /**
     * @brief Generates pseudo-legal moves that COULD get the king out of check
     * 
     * @param tBoard The position from wich moves are computed
     * @param outList Reference to a vector to wich the moves will be appended
     * @return Nothing
     */
    inline void evasions(const Board& tBoard, std::vector<Move> &outList) const{
        int kingSquare = tBoard.getKingSquare(tBoard.getSideToMove());
        uint64_t occupied = tBoard.getBitboard(white) | tBoard.getBitboard(black);
        uint64_t target = mLookup.getAttacks(queen, kingSquare, occupied) | mLookup.getAttacks(knight, kingSquare, occupied);
        generate(target, tBoard, outList);        
    }

    /**
     * @brief Checks if a square is attacked
     * 
     * @param tBoard The position to reference
     * @param tSquare The square in question
     * @param tSide Player color that's attacking
     * @return true if the square is under attack, false otherwise
     */
    bool isAttacked(const Board &tBoard, int tSquare, int tSide) const;

    /**
     * @brief Checks pseudo-legality of a move
     * 
     * @param tBoard The position to reference
     * @param tMove The move to validate
     * @return true if the move is pseudo-legal, false otherwise
     */
    bool validate(const Board& tBoard,const Move tMove) const;
private:
    void generate (uint64_t tTarget, const Board& tBoard, std::vector<Move>& outList) const;
    void pieceMoves(uint64_t tTarget, int tPiece, std::vector<Move>& tList, const Board& tBoard) const;
    void pawnMoves(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
    
    void castles(std::vector<Move> &tList, const Board &tBoard) const; 
    void enPassants(uint64_t tTarget, std::vector<Move> &tList, const Board &tBoard) const;
private:
    const MagicBitboards& mLookup;
};