#include "Board.hpp"
#include "utils.hpp"
#include <cassert>
#include <bitset>

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
    if (tolower(sideToMove[0]) != 'w') toggleSideToMove();

    // Set castling rights
    for (char character : castleRights) switch (character)
    {
    case 'k': m_stateHist.back() |= uint32_t(1) << 2; break;
    case 'q': m_stateHist.back() |= uint32_t(1) << 4; break;
    case 'K': m_stateHist.back() |= uint32_t(1) << 1; break;
    case 'Q': m_stateHist.back() |= uint32_t(1) << 3; break;
    }


    // Set ep square
    if (epSquare.compare("-") != 0) setEpSquare(atoi(epSquare.c_str()));

    // Set HMC
    for (int i = 0; i < atoi(halfmoveClock.c_str()); i ++) incrementHMC();

    // Set material count
    m_materialCount = 0;
    static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    for (int piece = pawn; piece < king; piece ++) m_materialCount += popCount(m_bitboard[piece]) * pieceVal[piece];
}

Board::Board(const Board &t_other) : m_bitboard{t_other.m_bitboard}, m_materialCount{t_other.m_materialCount}
{
    m_stateHist.emplace_back(t_other.m_stateHist.back());
}

Board &Board::operator=(const Board &t_other)
{
    if(this != &t_other){
        m_bitboard = t_other.m_bitboard;
        m_materialCount = t_other.m_materialCount;
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

uint32_t Board::getHash() const
{
    uint32_t key = 0;
    for(uint64_t bitboard : m_bitboard) key ^= hash32(uint32_t(bitboard)) ^ hash32(uint32_t(bitboard >> 32));
    key ^= uint32_t(hash32(m_stateHist.back()));

    return key;
}

void Board::makeMove(const Move &t_move)
{
    // ALWAYS removes en-passant and en-passant square
    // removes castling depending on the move type
    static constexpr uint32_t stateMask[64] = {
        0xffffe017, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe015, 0xffffe01f, 0xffffe01f, 0xffffe01d,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe01f,
        0xffffe00f, 0xffffe01f, 0xffffe01f, 0xffffe01f, 0xffffe00b, 0xffffe01f, 0xffffe01f, 0xffffe01b
    };
    uint64_t moveMask = uint64_t(1) << t_move.startingSquare() | uint64_t(1) << t_move.endSquare();
    int movedPiece, capturedPiece;
    int sideToMove = getSideToMove();

    m_stateHist.emplace_back(m_stateHist.back());
    m_stateHist.back() &= stateMask[t_move.startingSquare()] & stateMask[t_move.endSquare()];
    switch (t_move.flag()){
    case quiet:
        movedPiece = searchMoved(moveMask);
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[movedPiece] ^= moveMask;        
        if (movedPiece == pawn) resetHMC();
        else {
            incrementHMC();
            if (movedPiece == king) setKingSquare(sideToMove, t_move.endSquare());
        }
        break;
    case doublePush:
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= moveMask;
        setEpSquare(t_move.endSquare());
        resetHMC();
        break;
    case kingCastle:
        static constexpr uint64_t kingCastleMask[2] = {uint64_t(0x00000000000000a0), uint64_t(0xa000000000000000)};
        m_bitboard[sideToMove] ^= kingCastleMask[sideToMove] | moveMask;
        m_bitboard[king] ^= moveMask;
        m_bitboard[rook] ^= kingCastleMask[sideToMove];
        setKingSquare(sideToMove, t_move.endSquare());
        incrementHMC();
        break;
    case queenCastle:
        static constexpr uint64_t queenCastleMask[2] = {uint64_t(0x0000000000000009), uint64_t(0x0900000000000000)};
        m_bitboard[sideToMove] ^= queenCastleMask[sideToMove] | moveMask;
        m_bitboard[king] ^= moveMask;
        m_bitboard[rook] ^= queenCastleMask[sideToMove];
        setKingSquare(sideToMove, t_move.endSquare());
        incrementHMC();
        break;
    case capture:
        movedPiece = searchMoved(moveMask);
        capturedPiece = searchCaptured(moveMask);
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[movedPiece] ^= moveMask;
        m_bitboard[1 - sideToMove] ^= uint64_t(1) << t_move.endSquare();
        m_bitboard[capturedPiece]  ^= uint64_t(1) << t_move.endSquare();
        if (movedPiece == king) setKingSquare(sideToMove, t_move.endSquare());
        decreaseMaterialCount(capturedPiece);
        setCaptured(capturedPiece);
        resetHMC();
        break;
    case enPassant:
        static constexpr int offset[2] = {-8, 8};
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= moveMask;
        m_bitboard[1-sideToMove] ^= uint64_t(1) << (t_move.endSquare() + offset[sideToMove]);
        m_bitboard[pawn] ^= uint64_t(1) << (t_move.endSquare() + offset[sideToMove]);
        decreaseMaterialCount(pawn);
        setCaptured(pawn);
        resetHMC();
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        capturedPiece = searchCaptured(moveMask);
        m_bitboard[1 - sideToMove] ^= uint64_t(1) << t_move.endSquare();
        m_bitboard[capturedPiece]  ^= uint64_t(1) << t_move.endSquare();
        decreaseMaterialCount(capturedPiece);
        setCaptured(capturedPiece);
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= uint64_t(1) << t_move.startingSquare();
        m_bitboard[t_move.promoPiece()] ^= uint64_t(1) << t_move.endSquare();
        decreaseMaterialCount(pawn);
        increaseMaterialCount(t_move.promoPiece());
        resetHMC();
        break;
    }

    toggleSideToMove();
}

void Board::undoMove(const Move &t_move)
{
    uint64_t moveMask = uint64_t(1) << t_move.startingSquare() | uint64_t(1) << t_move.endSquare();
    int movedPiece, capturedPiece;
    toggleSideToMove();
    int sideToMove = getSideToMove();

    
    switch (t_move.flag()){
    case quiet:
        movedPiece = searchMoved(moveMask);
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[movedPiece] ^= moveMask;
        break;
    case doublePush:
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= moveMask;
        break;
    case kingCastle:
        static constexpr uint64_t kingCastleMask[2] = {uint64_t(0x00000000000000a0), uint64_t(0xa000000000000000)};
        m_bitboard[sideToMove] ^= kingCastleMask[sideToMove] | moveMask;
        m_bitboard[king] ^= moveMask;
        m_bitboard[rook] ^= kingCastleMask[sideToMove];
        break;
    case queenCastle:
        static constexpr uint64_t queenCastleMask[2] = {uint64_t(0x0000000000000009), uint64_t(0x0900000000000000)};
        m_bitboard[sideToMove] ^= queenCastleMask[sideToMove] | moveMask;
        m_bitboard[king] ^= moveMask;
        m_bitboard[rook] ^= queenCastleMask[sideToMove];
        break;
    case capture:
        movedPiece = searchMoved(moveMask);
        capturedPiece = getCaptured();
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[movedPiece] ^= moveMask;
        m_bitboard[1 - sideToMove] ^= uint64_t(1) << t_move.endSquare();
        m_bitboard[capturedPiece]  ^= uint64_t(1) << t_move.endSquare();
        increaseMaterialCount(capturedPiece);
        break;
    case enPassant:
        static constexpr int offset[2] = {-8, 8};
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= moveMask;
        m_bitboard[1-sideToMove] ^= uint64_t(1) << (t_move.endSquare() + offset[sideToMove]);
        m_bitboard[pawn] ^= uint64_t(1) << (t_move.endSquare() + offset[sideToMove]);
        increaseMaterialCount(pawn);
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        capturedPiece = getCaptured();
        m_bitboard[1 - sideToMove] ^= uint64_t(1) << t_move.endSquare();
        m_bitboard[capturedPiece]  ^= uint64_t(1) << t_move.endSquare();
        increaseMaterialCount(capturedPiece);
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        m_bitboard[sideToMove] ^= moveMask;
        m_bitboard[pawn] ^= uint64_t(1) << t_move.startingSquare();
        m_bitboard[t_move.promoPiece()] ^= uint64_t(1) << t_move.endSquare();
        increaseMaterialCount(pawn);
        decreaseMaterialCount(t_move.promoPiece());
        resetHMC();
        break;
    }

    m_stateHist.pop_back();
}

int Board::searchMoved(const uint64_t &t_moveMask) const
{
    for (int piece = pawn; piece <= king; piece ++) 
        if (m_bitboard[piece] & m_bitboard[getSideToMove()] & t_moveMask) return piece;
    throw std::runtime_error("moved piece not found");
}

int Board::searchCaptured(const uint64_t &t_moveMask) const
{
    for (int piece = queen; piece >= pawn; piece--) 
        if (m_bitboard[piece] & m_bitboard[1 - getSideToMove()] & t_moveMask) return piece;
    throw std::runtime_error("moved piece not found");
}

