#pragma once

#include <iostream>
#include <cstdint>

#include "notation.hpp"

class Move{
public:
    Move(): mMove{0U}{}
    Move(const Move &);
    Move(int tFrom, int tTo, int tFlag);

    void operator= (Move otherObj);
    friend bool operator== (const Move& thisObj, const Move& otherObj);
    friend bool operator!= (const Move& thisObj, const Move& otherObj);
    friend std::ostream& operator<< (std::ostream& os, const Move& cm);

    std::string asString() const;

    inline int to()  const {return mMove & 0x3f;}
    inline int from() const {return (mMove >> 6) & 0x3f;}
    inline int flag()       const {return (mMove >> 12) & 0x0f;}
    inline int asInt()      const {return mMove;}
    inline int promoPiece() const {return (flag() & 0x03) + knight;}

    inline bool isInit()       const {return asInt();}
    inline bool isCapture()    const {return (flag() & 0x04) != 0;}
    inline bool isDoublePush() const {return flag() == doublePush;}
    inline bool isPromo()      const {return (flag() & 0x08) != 0;}
    inline bool isEnPassant()  const {return flag() == enPassant;}
    inline bool isCastle()     const {return (flag() == kingCastle) || (flag() == queenCastle);}

private:
    uint16_t mMove;
};