#pragma once

#include <iostream>
#include <cstdint>

#include "notation.hpp"

class Move{
public:
    Move(): m_move{0U}{}
    Move(const Move &);
    Move(int t_from, int t_to, int t_flag, int t_piece);
    Move(int t_from, int t_to, int t_flag, int t_piece, int t_captured);

    void operator= (Move otherObj);
    friend bool operator== (const Move& thisObj, const Move& otherObj);
    friend bool operator!= (const Move& thisObj, const Move& otherObj);
    friend std::ostream& operator<< (std::ostream& os, const Move& cm);

    inline int endSquare()  const {return m_move & 0x3f;}
    inline int startingSquare() const {return (m_move >> 6) & 0x3f;}
    inline int flag()       const {return (m_move >> 12) & 0x0f;}
    inline int piece()      const {return (m_move >> 16) & 0x07;}
    inline int captured()   const {return (m_move >> 19) & 0x07;}
    inline int mvvLva()     const {return int8_t((m_move >> 22) & 0xff);}
    inline int asInt()      const {return m_move;}
    inline int promoPiece() const {return (flag() & 0x03) + knight;}

    inline bool isInit()       const {return asInt();}
    inline bool isCapture()    const {return (flag() & 0x04) != 0;}
    inline bool isDoublePush() const {return flag() == doublePush;}
    inline bool isPromo()      const {return (flag() & 0x08) != 0;}
    inline bool isEnPassant()  const {return flag() == enPassant;}
    inline bool isCastle()     const {return (flag() == kingCastle) || (flag() == queenCastle);}

private:
    uint32_t m_move;
};