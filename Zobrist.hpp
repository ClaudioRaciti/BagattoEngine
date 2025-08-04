#pragma once

#include <array>
#include <cstdint>
#include <random>

class Zobrist
{
public:
    // Deleted methods for singleton pattern
    Zobrist(const Zobrist&)             =delete;
    Zobrist& operator=(const Zobrist&)  =delete;

    static const Zobrist& getInstance();

    static constexpr int PIECE_OFFSET[7] = {0, 0, 64, 128, 192, 256, 320};
    static constexpr int SIDE_OFFSET[2] = { 0, 384 };
    inline uint64_t getPieceKey(int tSTM, int tPiece, int tSquare) const {
        return mPieceKeys[SIDE_OFFSET[tSTM] + PIECE_OFFSET[tPiece] + tSquare];
    }
    
    inline uint64_t getCastleKey(int tCastleFlag) const {return mCastleKeys[tCastleFlag];}
    inline uint64_t getEPKey(int tEPFile) const {return mEPKeys[tEPFile];}
    inline uint64_t getSTMKey() const {return mSTMKey;}

private:

    Zobrist();
    ~Zobrist() {delete mInstance; mInstance = nullptr;}

    void initPieces();
    void initCastle();
    void initEP();
    void initSideToMove();

private:
    static Zobrist* mInstance;

    std::array<uint64_t, 768> mPieceKeys;
    std::array<uint64_t, 16> mCastleKeys;
    std::array<uint64_t, 8> mEPKeys;
    uint64_t mSTMKey;

    std::mt19937_64 mRng;
};
