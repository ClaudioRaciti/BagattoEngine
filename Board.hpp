#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include "Move.hpp"
#include <cassert>

class Board
{
public:
    Board() = default;
    Board(std::string t_FEN);
    Board(const Board&);
    Board& operator= (const Board&);
    bool operator==(const Board&) const;
    bool operator!=(const Board&) const;
    
    inline uint64_t getBitboard(int t_piece) const {return m_bitboard[t_piece];}   


    inline int  getSideToMove() const               {return m_stateHist.back() & 0x1;}
    inline bool getShortCastle (int t_side) const   {return (m_stateHist.back() >> (t_side + 1)) & 0x1;}
    inline bool getLongCastle (int t_side) const    {return (m_stateHist.back() >> (t_side + 3)) & 0x1;}
    inline bool getEpState() const                  {return (m_stateHist.back() >> 5) & 0x01;}
    inline int  getEpSquare() const                 {return ((m_stateHist.back() >> 6) & 0xf) + 24;}
    inline int  getCaptured() const                 {return (m_stateHist.back() >> 10) & 0x7;}
    inline int  getKingSquare (int t_side) const    {return (m_stateHist.back() >> (13 + 6 * t_side)) & 0x3f;}
    inline int  getHMC() const                      {return (m_stateHist.back() >> 25) & 0x7f;} 

    inline int getMaterialCount() const {return m_materialCount;}

    uint32_t getHash() const;

    void makeMove(const Move &t_move);
    void undoMove(const Move &t_move);

    int searchMoved(const uint64_t &t_moveMask) const;
    int searchCaptured(const uint64_t &t_moveMask) const;

private:
    
    inline void toggleSideToMove()              {m_stateHist.back() ^= 0x01;}
    inline void removeShortCastle (int t_side)  {m_stateHist.back() &= ~(0x1 << (t_side + 1));}
    inline void removeLongCastle (int t_side)   {m_stateHist.back() &= ~(0x1 << (t_side + 3));}
    inline void setEpSquare (int t_square )     {m_stateHist.back() |= (1 << 5) | ((t_square - 24)  << 6);}
    inline void setCaptured (int t_piece)       {m_stateHist.back() |= t_piece << 10;}
    inline void setKingSquare (int t_side, int t_square) {
                                                m_stateHist.back() &= ~(0x3f << (13 + 6 * t_side));
                                                m_stateHist.back() |= t_square  << (13 + 6 * t_side);
                                            }
    inline void incrementHMC()                  {m_stateHist.back() += 0x1 << 25;}
    inline void resetHMC()                      {m_stateHist.back() &= ~(0x7f << 25);}

    inline void decreaseMaterialCount(int piece) {
        static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
        m_materialCount -= pieceVal[piece];
    }
    inline void increaseMaterialCount(int piece) {
        static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000}; 
        m_materialCount += pieceVal[piece];
    }


private:
    std::array<uint64_t, 8> m_bitboard;
    std::vector<uint32_t> m_stateHist;
    int m_materialCount;

    // stateHist entries are 32 bits arranged like:
    //
    // what i want is [srrrrEeeeewwwwwwbbbbbb5555555ccc]
    // what i got is  [srrrrEeeee??wwwwwwbbbbbb5555555?]
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