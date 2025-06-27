#pragma once

#include <array>
#include <cstdint>
#include <random>

#include "notation.hpp"

class Zobrist
{
public:
    // Deleted methods for singleton pattern
    Zobrist(const Zobrist&)             =delete;
    Zobrist& operator=(const Zobrist&)  =delete;

    static const Zobrist& getInstance();

    inline uint64_t getPieceKey(int sideToMove, int piece, int square) const {return m_pieceKey[getIndex(sideToMove, piece, square)];}
    inline uint64_t getCastleKey(int castleFlag) const {return m_castleKey[castleFlag];}
    inline uint64_t getEPKey(int EPFile) const {return m_EPKey[EPFile];}
    inline uint64_t getSTMKey() const {return m_STMKey;}

private:

    Zobrist();
    ~Zobrist() {delete m_instance; m_instance = nullptr;}

    void initPieces();
    void initCastle();
    void initEP();
    void initSideToMove();

    inline int getIndex(int sideToMove, int piece, int square) const {return 384 * sideToMove + 64 * (piece - pawn) + square;}

private:
    static Zobrist* m_instance;

    std::array<uint64_t, 768> m_pieceKey;
    std::array<uint64_t, 16> m_castleKey;
    std::array<uint64_t, 8> m_EPKey;
    uint64_t m_STMKey;

    std::mt19937_64 rng;
    std::uniform_int_distribution<uint64_t> device;
};
