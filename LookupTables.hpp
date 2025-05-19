#pragma once

#include <cstdint>

class LookupTables{
public:
    // Deleted methods for singleton pattern
    LookupTables(const LookupTables&)               = delete;
    LookupTables& operator=(const LookupTables&)    = delete;

    static const LookupTables& getInstance();

    constexpr uint64_t rayAttacks   (int t_square, int t_direction)     const {return m_rayAttacks[t_square][t_direction];}
    constexpr uint64_t knightAttacks(int t_square) const {return m_knightAttacks[t_square];}
    constexpr uint64_t kingAttacks  (int t_square) const {return m_kingAttacks[t_square];}
    constexpr uint64_t rookAttacks  (uint64_t t_occupied, int t_square) const {return m_rMagicDb[t_square][((t_occupied & m_rMask[t_square]) * m_rMagic[t_square]) >> m_rShift[t_square]];}
    constexpr uint64_t bishopAttacks(uint64_t t_occupied, int t_square) const {return m_bMagicDb[t_square][((t_occupied & m_bMask[t_square]) * m_bMagic[t_square]) >> m_bShift[t_square]];}
    constexpr uint64_t queenAttacks (uint64_t t_occupied, int t_square) const {return rookAttacks(t_occupied, t_square) | bishopAttacks(t_occupied, t_square);}
    constexpr uint64_t pawnAttacks(int t_square, int t_sideToMove) const {return m_pawnAttacks[t_square][t_sideToMove];}

    constexpr uint64_t getAttacks(int t_piece, int t_square, uint64_t t_occupied) const {
        switch (t_piece) {
        case 3: return knightAttacks(t_square); // Knight
        case 4: return bishopAttacks(t_occupied, t_square); // Bishop
        case 5: return rookAttacks(t_occupied, t_square);   // Rook
        case 6: return queenAttacks(t_occupied, t_square);  // Queen
        case 7: return kingAttacks(t_square);   // King
        }
        return 0;
    }

private:
    LookupTables();
    ~LookupTables() = default;

    void initRayAttacks();
    void initKnightAttacks();
    void initKingAttacks();
    void initPawnAttacks();
    void initMagicMoves();

    uint64_t initMagicOcc(int *, int, uint64_t);
    uint64_t initMagicBMoves(int, uint64_t);
    uint64_t initMagicRMoves(int, uint64_t);

private:
    static LookupTables *m_instance;

    uint64_t m_rayAttacks[64][8]; 
    uint64_t m_knightAttacks[64];
    uint64_t m_kingAttacks[64];
    uint64_t m_pawnAttacks[64][2];

    const unsigned int m_rShift[64];
    const uint64_t m_rMask[64];
    const uint64_t m_rMagic[64];
    const unsigned int m_bShift[64];
    const uint64_t m_bMask[64];
    const uint64_t m_bMagic[64];

    uint64_t m_rMagicDb[64][1<<12];
    uint64_t m_bMagicDb[64][1<<9];
};