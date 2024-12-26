#include "Board.hpp"

Board::Board()
{
    m_stateHist.reserve(20);
    m_stateHist.emplace_back(uint32_t(0x1e));
}

void Board::makeMove(const Move &t_move)
{
    // ALWAYS removes en-passant
    // removes castling depending on the move type
    static constexpr uint32_t stateMask[64] = {
        0xffffffd7, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffd5, 0xffffffdf, 0xffffffdf, 0xffffffdd,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffdf,
        0xffffffcf, 0xffffffdf, 0xffffffdf, 0xffffffdf, 0xffffffcb, 0xffffffdf, 0xffffffdf, 0xffffffdb
    };
    m_stateHist.emplace_back(m_stateHist.back());
    m_stateHist.back() &= stateMask[t_move.startingSquare()] & stateMask[t_move.endSquare()];
    if (t_move.isCapture() || t_move.piece() == pawn) resetHMC(); else incrementHMC();
    updateBitboards(t_move);
    toggleSideToMove();
}

void Board::undoMove(const Move &t_move)
{
    toggleSideToMove();
    updateBitboards(t_move);
    m_stateHist.pop_back();
}

void Board::updateBitboards(const Move &t_move)
{
    uint64_t endSqMask = uint64_t(1) << t_move.endSquare();
    uint64_t moveMask = (uint64_t(1) << t_move.startingSquare()) | endSqMask;   
    m_bitboard[getSideToMove()] ^= moveMask;
    m_bitboard[t_move.piece()] ^= moveMask;

    switch (t_move.flag()){
    case quiet:
        break;
    case doublePush:
        setEpState(true);
        setEpSquare(t_move.endSquare());
        break;
    case kingCastle:
        static constexpr uint64_t rookMask[2] = {uint64_t(0x00000000000000a0), uint64_t(0xa000000000000000)};
        m_bitboard[getSideToMove()] ^= rookMask[getSideToMove()];
        m_bitboard[rook] ^= rookMask[getSideToMove()];
        break;
    case queenCastle:
        static constexpr uint64_t rookMask[2] = {uint64_t(0x0000000000000009), uint64_t(0x0900000000000000)};
        m_bitboard[getSideToMove()] ^= rookMask[getSideToMove()];
        m_bitboard[rook] ^= rookMask[getSideToMove()];
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
