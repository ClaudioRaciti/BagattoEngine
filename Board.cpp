#include "Board.hpp"
#include "utils.hpp"
#include <cassert>
#include <bitset>


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
    m_stateHist.emplace_back(uint32_t(0x1e));
    setKingSquare(white, e1);
    setKingSquare(black, e8);
}

Board::Board(std::string t_FEN)
{
    std::vector<std::string> dataFields = split(t_FEN, ' ');
    std::string boardField = dataFields[0];
    std::string sideToMove = dataFields[1];
    std::string castleRights = dataFields[2];
    std::string epSquare = dataFields[3];
    std::string halfmoveClock = dataFields.size() < 5 ? "0" : dataFields[4];

    m_stateHist.emplace_back(uint32_t(0x0));

    // Set bitboards to zero
    for(int i = 0; i < 8; i ++) m_bitboard[i] = 0ULL;

    // Setup bitboards:
    std::vector<std::string> boardSquares = split(boardField, '/');
    for (int rank = 0; rank < 8; rank++){
        int stringIndex = 0;
        for (int file = 0; file < 8; file ++){
            char squareContent = boardSquares[7 - rank][stringIndex];
            stringIndex +=1;
            int squareNumber = (rank * 8) + file;
            if (isdigit(squareContent)) file += squareContent - '1';
            else switch (squareContent)
            {
            case 'p': m_bitboard[pawn]  |= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'P': m_bitboard[pawn]  |= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            case 'n': m_bitboard[knight]|= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'N': m_bitboard[knight]|= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            case 'b': m_bitboard[bishop]|= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'B': m_bitboard[bishop]|= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            case 'r': m_bitboard[rook]  |= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'R': m_bitboard[rook]  |= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            case 'q': m_bitboard[queen] |= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'Q': m_bitboard[queen] |= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            case 'k': m_bitboard[king]  |= uint64_t(1) << squareNumber; m_bitboard[black] |= uint64_t(1) << squareNumber; break;
            case 'K': m_bitboard[king]  |= uint64_t(1) << squareNumber; m_bitboard[white] |= uint64_t(1) << squareNumber; break;
            }
        }
    }

    setKingSquare(white, bitScanForward(m_bitboard[king] & m_bitboard[white]));
    setKingSquare(black, bitScanForward(m_bitboard[king] & m_bitboard[black]));

    // Set side to move
    tolower(sideToMove[0]) == 'w' ? m_stateHist.back() &= ~0x1 : m_stateHist.back() |= 0x1;

    // Set castling rights
    for (char character : castleRights) switch (character)
    {
    case 'k': m_stateHist.back() |= uint32_t(1) << 2; break;
    case 'q': m_stateHist.back() |= uint32_t(1) << 4; break;
    case 'K': m_stateHist.back() |= uint32_t(1) << 1; break;
    case 'Q': m_stateHist.back() |= uint32_t(1) << 3; break;
    }


    // Set ep square
    if (epSquare.compare("-") != 0) m_stateHist.back() |= (1 << 5) | ((1 << atoi(epSquare.c_str())) << 6);

    // Set HMC
    m_stateHist.back() += atoi(halfmoveClock.c_str()) << 24;
}

Board::Board(const Board &t_other) : m_bitboard{t_other.m_bitboard}
{
    m_stateHist.emplace_back(t_other.m_stateHist.back());
}

Board &Board::operator=(const Board &t_other)
{
    if(this != &t_other){
        m_bitboard = t_other.m_bitboard;
        m_stateHist.emplace_back(t_other.m_stateHist.back());
    }
    return *this;
}

bool Board::operator==(const Board &t_other) const
{
    return (m_bitboard == t_other.m_bitboard) && (m_stateHist.back() == t_other.m_stateHist.back());
}

bool Board::operator!=(const Board &t_other) const
{
    return (m_bitboard != t_other.m_bitboard) || (m_stateHist.back() != t_other.m_stateHist.back());
}

uint64_t Board::getHash() const
{
    uint64_t key = 0ULL;
    for(uint64_t bitboard : m_bitboard) key ^= hash64(bitboard);
    key ^= uint64_t(hash32(m_stateHist.back())) << 32;

    return key;
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
