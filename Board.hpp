#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include "Move.hpp"
#include "Zobrist.hpp"
#include <cassert>

class Board
{
public:
    Board(): mZobrist{Zobrist::getInstance()}{}
    Board(std::string tFEN);
    Board(const Board&);
    Board& operator= (const Board&);
    bool operator==(const Board&) const;
    bool operator!=(const Board&) const;
    inline friend std::ostream& operator<< (std::ostream& os, const Board& cb) {return os << cb.asString();}
    std::string asString() const;
    
    
    inline uint64_t getBitboard(int tPiece) const {return mBitboards[tPiece];}   

    inline int  getSideToMove() const           {return mStateHist.back() & 0x1;}
    inline int  getCastles() const              {return (mStateHist.back() >> 1) & 0xf;}
    inline bool getShortCastle (int tSide)const {return (mStateHist.back() >> (tSide + 1)) & 0x1;}
    inline bool getLongCastle (int tSide) const {return (mStateHist.back() >> (tSide + 3)) & 0x1;}
    inline bool getEpState() const              {return (mStateHist.back() >> 5) & 0x01;}
    inline int  getEpSquare() const             {return ((mStateHist.back() >> 6) & 0xf) + 24;}
    inline int  getCaptured() const             {return (mStateHist.back() >> 10) & 0x7;}
    inline int  getKingSquare (int tSide) const {return (mStateHist.back() >> (13 + 6 * tSide)) & 0x3f;}
    inline int  getHMC() const                  {return (mStateHist.back() >> 25) & 0x7f;} 
    inline int  getFMC() const                  {return (mStateHist.size() / 2);}

    inline uint64_t getHash() const {return mKey;}

    void makeMove(const Move &tMove);
    void undoMove(const Move &tMove);

    inline int searchPiece(int tSquare) const {return mPieceSquare[tSquare];}

private:
    inline void movePiece(int tSTM, int tPiece, int tFrom, int tTo){
        const uint64_t mask = 1ULL << tFrom | 1ULL << tTo;
        mBitboards[tPiece] ^= mask;
        mBitboards[tSTM]   ^= mask;
        mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tFrom);
        mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tTo);
    }
    inline void capturePiece(int tSTM, int tPiece, int tSquare){
        const uint64_t mask = 1ULL << tSquare;
        mBitboards[tPiece] ^= mask;
        mBitboards[1-tSTM] ^= mask;
        mKey ^= mZobrist.getPieceKey(1-tSTM, tPiece, tSquare);
    }
    inline void promotePiece(int tSTM, int tPiece, int tFrom, int tTo){
        const uint64_t maskTo = 1ULL << tTo;
        const uint64_t maskFrom = 1ULL << tFrom;
        mBitboards[pawn]  ^= maskFrom;
        mBitboards[tPiece] ^= maskTo;
        mBitboards[tSTM]   ^= maskFrom | maskTo;
        mKey ^= mZobrist.getPieceKey(tSTM, pawn, tFrom);
        mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tTo);
    }

    inline void toggleSideToMove()              {mStateHist.back() ^= 0x01; mKey ^= mZobrist.getSTMKey();}
    inline void removeShortCastle (int tSide)   {mStateHist.back() &= ~(0x1 << (tSide + 1));}
    inline void removeLongCastle (int tSide)    {mStateHist.back() &= ~(0x1 << (tSide + 3));}
    inline void setEpSquare (int tSquare )      {mStateHist.back() |= (1 << 5) | ((tSquare % 24)  << 6);}
    inline void setCaptured (int tPiece)        {mStateHist.back() |= tPiece << 10;}
    inline void setKingSquare (int tSide, int tSquare) {
                                                mStateHist.back() &= ~(0x3f << (13 + 6 * tSide));
                                                mStateHist.back() |= tSquare  << (13 + 6 * tSide);
                                            }
    inline void incrementHMC()                  {mStateHist.back() += 0x1 << 25;}
    inline void resetHMC()                      {mStateHist.back() &= ~(0x7f << 25);}

private:
    std::array<uint64_t, 8> mBitboards;
    std::array<int, 64> mPieceSquare;
    std::vector<uint32_t> mStateHist;
    uint64_t mKey = 0ULL;

    const Zobrist& mZobrist;

    // stateHist entries are 32 bits arranged like:
    // what i want is [srrrrEeeeecccwwwwwwbbbbbb5555555]
    //
    // Where:
    // first 1 bit for side to move     [s]
    // then  4 bits for castling rights [r]
    // then  1 bit for en-passant state [E]
    // then  4 bits for ep square       [e]
    // then  6 bits for white king pos  [w]
    // then  6 bits for black king pos  [b]
    // then  7 bits for 50 move rule    [5]
    // last  3 bits for captured piece  [c]
};