#pragma once

#include <cstdint>
#include <stdexcept>
#include "notation.hpp"

class MagicBitboards{
public:
    // Deleted methods for singleton pattern
    MagicBitboards(const MagicBitboards&)               = delete;
    MagicBitboards& operator=(const MagicBitboards&)    = delete;

    static const MagicBitboards& getInstance();

    /**
     * @brief Returns attack pattern for pawns
     * 
     * @param tSquare Where the pawn is located
     * @param tSide Pawn color
     * @return uint64_t attack pattern bitboard
     */
    constexpr uint64_t pawnAttacks(int tSquare, int tSide) const {
        return mPawnAttacks[tSquare][tSide];
    }

    /**
     * @brief Returns attack pattern for (non-pawn) pieces 
     * 
     * @param tPiece Non-pawn piece 
     * @param tSquare Where the piece is located
     * @param tOccupied Position occupancy bitboard
     * @return uint64_t attack pattern bitboard
     */
    constexpr uint64_t getAttacks(int tPiece, int tSquare, uint64_t tOccupied) const {
        switch (tPiece) {
            case knight: return knightAttacks(tSquare);
            case bishop: return bishopAttacks(tOccupied, tSquare);
            case rook:   return rookAttacks(tOccupied, tSquare);
            case queen:  return queenAttacks(tOccupied, tSquare);
            case king:   return kingAttacks(tSquare);
            default: throw std::invalid_argument("Input piece is invalid");
        }
    }

private:
    MagicBitboards();
    ~MagicBitboards() {delete mInstance; mInstance = nullptr;}

    void initRayAttacks();
    void initKnightAttacks();
    void initKingAttacks();
    void initPawnAttacks();
    void initMagicMoves();
    
    constexpr uint64_t rayAttacks (int tSquare, int tDirection) const {
        return mRayAttacks[tSquare][tDirection];
    }
    constexpr uint64_t knightAttacks (int tSquare) const {
        return mKnightAttacks[tSquare];
    }
    constexpr uint64_t kingAttacks (int tSquare) const {
        return mKingAttacks[tSquare];
    }
    constexpr uint64_t rookAttacks (uint64_t tOccupied, int tSquare) const {
        return mRMagicDb[tSquare][((tOccupied & mRMask[tSquare]) * mRMagic[tSquare]) >> mRShift[tSquare]];
    }
    constexpr uint64_t bishopAttacks (uint64_t tOccupied, int tSquare) const {
        return mBMagicDb[tSquare][((tOccupied & mBMask[tSquare]) * mBMagic[tSquare]) >> mBShift[tSquare]];
    }
    constexpr uint64_t queenAttacks (uint64_t tOccupied, int tSquare) const {
        return rookAttacks(tOccupied, tSquare) | bishopAttacks(tOccupied, tSquare);
    }

    uint64_t initMagicOcc(int *, int, uint64_t);
    uint64_t initMagicBMoves(int, uint64_t);
    uint64_t initMagicRMoves(int, uint64_t);

private:
    static MagicBitboards *mInstance;

    uint64_t mRayAttacks[64][8]; 
    uint64_t mKnightAttacks[64];
    uint64_t mKingAttacks[64];
    uint64_t mPawnAttacks[64][2];

    const unsigned int mRShift[64];
    const uint64_t mRMask[64];
    const uint64_t mRMagic[64];
    const unsigned int mBShift[64];
    const uint64_t mBMask[64];
    const uint64_t mBMagic[64];

    uint64_t mRMagicDb[64][1<<12];
    uint64_t mBMagicDb[64][1<<9];
};