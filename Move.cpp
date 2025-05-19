#include "Move.hpp"
#include <map>

Move::Move(const Move &t_ohter)
{
    m_move = t_ohter.m_move;
}

Move::Move(int t_from, int t_to, int t_flag, int t_piece)
{
    m_move = ((t_piece & 0x0f) << 16 | (t_flag & 0x0f) << 12 | (t_from & 0x3f) << 6 | (t_to & 0x3f));
}

Move::Move(int t_from, int t_to, int t_flag, int t_piece, int t_captured)
{
    static constexpr int pieceValue[8] = {0, 0, 1, 3, 3, 5, 10, 100};
    m_move =  (t_flag & 0x0f) << 12 | (t_from & 0x3f) << 6 | (t_to & 0x3f);
    m_move |= (t_captured & 0x0f) << 19 | (t_piece & 0x0f) << 16;
    m_move |= (pieceValue[t_captured] - pieceValue[t_piece]) << 22;
}

void Move::operator=(Move otherObj)
{
    m_move = otherObj.m_move;
}

bool operator==(const Move &thisObj, const Move &otherObj)
{
    return thisObj.m_move == otherObj.m_move;
}

bool operator!=(const Move &thisObj, const Move &otherObj)
{
    return thisObj.m_move != otherObj.m_move;
}

std::ostream &operator<<(std::ostream &os, const Move &cm)
{
    std::map<int, std::string> squares{
        { 0,"a1"},{ 1,"b1"},{ 2,"c1"},{ 3,"d1"},{ 4,"e1"},{ 5,"f1"},{ 6,"g1"},{ 7,"h1"},
        { 8,"a2"},{ 9,"b2"},{10,"c2"},{11,"d2"},{12,"e2"},{13,"f2"},{14,"g2"},{15,"h2"},
        {16,"a3"},{17,"b3"},{18,"c3"},{19,"d3"},{20,"e3"},{21,"f3"},{22,"g3"},{23,"h3"},
        {24,"a4"},{25,"b4"},{26,"c4"},{27,"d4"},{28,"e4"},{29,"f4"},{30,"g4"},{31,"h4"},
        {32,"a5"},{33,"b5"},{34,"c5"},{35,"d5"},{36,"e5"},{37,"f5"},{38,"g5"},{39,"h5"},
        {40,"a6"},{41,"b6"},{42,"c6"},{43,"d6"},{44,"e6"},{45,"f6"},{46,"g6"},{47,"h6"},
        {48,"a7"},{49,"b7"},{50,"c7"},{51,"d7"},{52,"e7"},{53,"f7"},{54,"g7"},{55,"h7"},
        {56,"a8"},{57,"b8"},{58,"c8"},{59,"d8"},{60,"e8"},{61,"f8"},{62,"g8"},{63,"h8"}
    };
    
    std::map<int, std::string> pieces{
        {0,"white"},{1,"black"},{2,"pawn"},{3,"n"},{4,"b"},{5,"r"},{6,"q"},{7,"k"}
    };

    std::map<int, std::string> flags{
        {0,"quiet"},{1,"double push"},{2,"kingside castle"},{3,"queenside castle"},
        {4,"capture"},{5,"en passant"},{8,"promotion to knight"},{9,"promotion to bishop"},
        {10,"promotion to rook"},{11,"promotion to queen"},{12,"capture and promotion\
        to knight"},{13,"capture and promotion to bishop"},{14,"capture and promotion\
        to rook"},{15,"capture and promotion to queen"}
    };

    os << squares[cm.startingSquare()] << squares[cm.endSquare()];
    if (cm.isPromo()) os << pieces[cm.promoPiece()];

    return os;
}
