#include "Board.hpp"

Board::Board()
{
    m_bitboard[white]   = (uint64_t) 0x000000000000ffff;
    m_bitboard[black]   = (uint64_t) 0xffff000000000000;
    m_bitboard[pawn]    = (uint64_t) 0x00ff00000000ff00;
    m_bitboard[knight]  = (uint64_t) 0x4200000000000042;
    m_bitboard[bishop]  = (uint64_t) 0x2400000000000024;
    m_bitboard[rook]    = (uint64_t) 0x8100000000000081;
    m_bitboard[queen]   = (uint64_t) 0x0800000000000008;
    m_bitboard[king]    = (uint64_t) 0x1000000000000010;

    // m_bitboard[white]   = (uint64_t) 0x000000181024ff91;
    // m_bitboard[black]   = (uint64_t) 0x917d730002800000;
    // m_bitboard[pawn]    = (uint64_t) 0x002d50081280e700;
    // m_bitboard[knight]  = (uint64_t) 0x0000221000040000;
    // m_bitboard[bishop]  = (uint64_t) 0x0040010000001800;
    // m_bitboard[rook]    = (uint64_t) 0x8100000000000081;
    // m_bitboard[queen]   = (uint64_t) 0x0010000000200000;
    // m_bitboard[king]    = (uint64_t) 0x1000000000000010;
    m_stateHist.reserve(20);
    m_stateHist.emplace_back(uint32_t(0x1e));
    setKingSquare(white, e1);
    setKingSquare(black, e8);
}

void Board::makeMove(const Move &t_move)
{
    // ALWAYS removes en-passant and en-passant square
    // removes castling depending on the move type
    static constexpr uint32_t stateMask[64] = {
        0xfffff017, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff015, 0xfffff01f, 0xfffff01f, 0xfffff01d,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff01f,
        0xfffff00f, 0xfffff01f, 0xfffff01f, 0xfffff01f, 0xfffff00b, 0xfffff01f, 0xfffff01f, 0xfffff01b
    };
    m_stateHist.emplace_back(m_stateHist.back());
    m_stateHist.back() &= stateMask[t_move.startingSquare()] & stateMask[t_move.endSquare()];
    if (t_move.isCapture() || t_move.piece() == pawn) resetHMC(); else incrementHMC();
    if (t_move.piece() == king) setKingSquare(getSideToMove(), t_move.endSquare());
    if (t_move.isDoublePush()) {m_stateHist.back() |= 1 << 5; m_stateHist.back() |= (t_move.endSquare() & 0x3f) << 6;}
    updateBitboards(t_move);
    toggleSideToMove();
}

void Board::undoMove(const Move &t_move)
{
    m_stateHist.pop_back();
    updateBitboards(t_move);
}

void Board::updateBitboards(const Move &t_move)
{
    uint64_t endSqMask = uint64_t(1) << t_move.endSquare();
    uint64_t moveMask = (uint64_t(1) << t_move.startingSquare()) | endSqMask;   
    m_bitboard[getSideToMove()] ^= moveMask;
    m_bitboard[t_move.piece()] ^= moveMask;

    switch (t_move.flag()){
    case kingCastle:
        static constexpr uint64_t kingCastleMask[2] = {uint64_t(0x00000000000000a0), uint64_t(0xa000000000000000)};
        m_bitboard[getSideToMove()] ^= kingCastleMask[getSideToMove()];
        m_bitboard[rook] ^= kingCastleMask[getSideToMove()];
        break;
    case queenCastle:
        static constexpr uint64_t queenCastleMask[2] = {uint64_t(0x0000000000000009), uint64_t(0x0900000000000000)};
        m_bitboard[getSideToMove()] ^= queenCastleMask[getSideToMove()];
        m_bitboard[rook] ^= queenCastleMask[getSideToMove()];
        break;
    case capture:
        m_bitboard[1-getSideToMove()] ^= endSqMask;
        m_bitboard[t_move.captured()] ^= endSqMask;
        break;
    case enPassant:
        static constexpr int offset[2] = {-8, 8};
        m_bitboard[1-getSideToMove()] ^= uint64_t(1) << (t_move.endSquare() + offset[getSideToMove()]);
        m_bitboard[pawn] ^= uint64_t(1) << (t_move.endSquare() + offset[getSideToMove()]);
        break;
    case knightPromo:
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[knight] ^= endSqMask;
        break;
    case bishopPromo:
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[bishop] ^= endSqMask;
        break;
    case rookPromo:
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[rook] ^= endSqMask;
        break;
    case queenPromo:
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[queen] ^= endSqMask;
        break;
    case knightPromoCapture:
        m_bitboard[1-getSideToMove()] ^= endSqMask;
        m_bitboard[t_move.captured()] ^= endSqMask;
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[knight] ^= endSqMask;
        break;
    case bishopPromoCapture:
        m_bitboard[1-getSideToMove()] ^= endSqMask;
        m_bitboard[t_move.captured()] ^= endSqMask;
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[bishop] ^= endSqMask;
        break;
    case rookPromoCapture:
        m_bitboard[1-getSideToMove()] ^= endSqMask;
        m_bitboard[t_move.captured()] ^= endSqMask;
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[rook] ^= endSqMask;
        break;
    case queenPromoCapture:
        m_bitboard[1-getSideToMove()] ^= endSqMask;
        m_bitboard[t_move.captured()] ^= endSqMask;
        m_bitboard[pawn] ^= endSqMask;
        m_bitboard[queen] ^= endSqMask;
        break;
    }
}
