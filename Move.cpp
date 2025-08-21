#include "Move.hpp"
#include <array>

Move::Move(const Move &tOther)
{
    mMove = tOther.mMove;
}

Move::Move(int tFrom, int tTo, int tFlag)
{
    mMove = (tFlag & 0x0f) << 12 | (tFrom & 0x3f) << 6 | (tTo & 0x3f);
}

void Move::operator=(Move tOther)
{
    mMove = tOther.mMove;
}

std::string Move::asString() const
{
    std::string output;

    static constexpr std::array<const char*, 64> squares = {
        "a1","b1","c1","d1","e1","f1","g1","h1",
        "a2","b2","c2","d2","e2","f2","g2","h2",
        "a3","b3","c3","d3","e3","f3","g3","h3",
        "a4","b4","c4","d4","e4","f4","g4","h4",
        "a5","b5","c5","d5","e5","f5","g5","h5",
        "a6","b6","c6","d6","e6","f6","g6","h6",
        "a7","b7","c7","d7","e7","f7","g7","h7",
        "a8","b8","c8","d8","e8","f8","g8","h8"
    };
    
    static constexpr std::array<char, 8> pieces{
        ' ',' ','p','n','b','r','q','k'
    };

    output += squares[from()];
    output += squares[to()];
    if (isPromo()) output += pieces[promoPiece()];

    return output;
}

bool operator==(const Move &tThis, const Move &tOther)
{
    return tThis.mMove == tOther.mMove;
}

bool operator!=(const Move &tThis, const Move &tOther)
{
    return tThis.mMove != tOther.mMove;
}

std::ostream &operator<<(std::ostream &os, const Move &cm)
{
    std::string output = cm.asString();
    os << output;

    return os;
}
