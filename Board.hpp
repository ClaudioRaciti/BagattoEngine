#pragma once

#include <cstdint>
#include <vector>
#include "Move.hpp"

class Board
{
public:
    Board();
    
    inline uint64_t getBitboard(int t_piece) const {return m_bitboard[t_piece];}   

    inline bool getShortCastle (int t_side) const {return (m_stateHist.back() >> (t_side + 1)) & 0x1;}
    inline bool getLongCastle  (int t_side) const {return (m_stateHist.back() >> (t_side + 3)) & 0x1;}
    inline int  getKingSquare  (int t_side) const {return (m_stateHist.back() >> (12 + 6 * t_side)) & 0x3f;}

    inline int  getSideToMove() const {return m_stateHist.back() & 0x1;}
    inline bool getEpState()    const {return (m_stateHist.back() >> 5) & 0x01;}
    inline int  getEpSquare()   const {return (m_stateHist.back() >> 6) & 0x3f;}
    inline int  getHMC()        const {return (m_stateHist.back() >> 24) & 0x7f;} 


    void makeMove(const Move &t_move);
    void undoMove(const Move &t_move);

private:
    inline void setKingSquare       (int t_side, int t_square) {m_stateHist.back() &= ~(0x3f << (12 + 6 * t_side));m_stateHist.back() |= (t_square & 0x3f) << (12 + 6 * t_side);}
    inline void removeShortCastle   (int t_side)    {m_stateHist.back() &= ~(0x1 << (t_side + 1));}
    inline void removeLongCastle    (int t_side)    {m_stateHist.back() &= ~(0x1 << (t_side + 3));}
    inline void toggleSideToMove()  {m_stateHist.back() ^= 0x01;}
    inline void incrementHMC()      {m_stateHist.back() += 0x1 << 24;}
    inline void resetHMC()          {m_stateHist.back() &= ~(0x7f << 24);}

    void updateBitboards(const Move &t_move);

private:
    uint64_t m_bitboard[8];
    std::vector<uint32_t> m_stateHist;
    // first 1 bit for side to move
    // then  4 bits for castling rights
    // then  1 bit for en-passant state
    // then  6 bits for ep square 
    // then  6 bits for white king pos
    // then  6 bits for black king pos
    // last  7 bits for 50 move rule
    // one remainig bit
    // space efficiency can be improved by using only 4 bits for ep square and the last remanining 3 bits for last captured piece
};