#pragma once

#include <iostream>
#include <cstdint>

#include "notation.hpp"

class Move{
public:
    Move(): m_move{0U}{}
    Move(const Move &);
    Move(int t_from, int t_to, int t_flag);

    void operator= (Move otherObj);
    friend bool operator== (const Move& thisObj, const Move& otherObj);
    friend bool operator!= (const Move& thisObj, const Move& otherObj);
    friend std::ostream& operator<< (std::ostream& os, const Move& cm);

    std::string asString() const;

    inline int to()  const {return m_move & 0x3f;}
    inline int from() const {return (m_move >> 6) & 0x3f;}
    inline int flag()       const {return (m_move >> 12) & 0x0f;}
    inline int asInt()      const {return m_move;}
    inline int promoPiece() const {return (flag() & 0x03) + knight;}

    inline bool isInit()       const {return asInt();}
    inline bool isCapture()    const {return (flag() & 0x04) != 0;}
    inline bool isDoublePush() const {return flag() == doublePush;}
    inline bool isPromo()      const {return (flag() & 0x08) != 0;}
    inline bool isEnPassant()  const {return flag() == enPassant;}
    inline bool isCastle()     const {return (flag() == kingCastle) || (flag() == queenCastle);}

private:
    uint16_t m_move;
};