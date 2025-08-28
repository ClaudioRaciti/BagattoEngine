#include "Board.hpp"
#include "notation.hpp"
#include "utils.hpp"
#include <cassert>
#include <cstdint>
#include <ostream>
#include <string>
#include <sstream>
#include <vector>
#include <array>



Board::Board(std::string tFEN) : mZobrist{Zobrist::getInstance()} {
    struct PieceColor { int pieceType; int pieceColor; };

    static constexpr std::array<PieceColor, 128> pieceColorMap = [] {
        std::array<PieceColor, 128> map {};
        auto insert = [&] (char c, int piece, int color) {
            map[static_cast<unsigned char>(c)] = {piece, color};
        };
        insert('P', pawn,   white); insert('p', pawn,   black);
        insert('N', knight, white); insert('n', knight, black);
        insert('B', bishop, white); insert('b', bishop, black);
        insert('R', rook,   white); insert('r', rook,   black);
        insert('Q', queen,  white); insert('q', queen,  black);
        insert('K', king,   white); insert('k', king,   black);
        return map;
    }();

    // Parse FEN fields
    std::istringstream fenStream(tFEN);
    std::string fenPiecePlacement, fenActiveColor, fenCastlingRights, fenEpSquare;
    int fenHalfmoveClock = 0;

    fenStream >> fenPiecePlacement >> fenActiveColor >> fenCastlingRights >> fenEpSquare;
    if (!(fenStream >> fenHalfmoveClock)) fenHalfmoveClock = 0;

    // Initialize state
    mStateHist.emplace_back(uint32_t(0x0));
    std::fill(std::begin(mBitboards), std::end(mBitboards), 0ULL);

    // Setup bitboards
    std::istringstream piecePlacementStream(fenPiecePlacement);
    std::string fenRankString;

    for (int rankIndex = 0; rankIndex < 8; ++rankIndex) {
        getline(piecePlacementStream, fenRankString, '/');
        int fileIndex = 0;
        for (char c : fenRankString) {
            if (isdigit(c)) {
                fileIndex += c - '0';
                continue;
            }
            int squareIndex = (7 - rankIndex) * 8 + fileIndex++;
            auto [piece, pieceColor] = pieceColorMap[static_cast<unsigned char>(c)];
            assert(piece);
            uint64_t squareMask = 1ULL << squareIndex;
            mBitboards[piece] |= squareMask;
            mBitboards[pieceColor] |= squareMask;
            mKey ^= mZobrist.getPieceKey(pieceColor, piece, squareIndex);
        }
    }

    // King positions
    setKingSquare(white, bitScanForward(mBitboards[king] & mBitboards[white]));
    setKingSquare(black, bitScanForward(mBitboards[king] & mBitboards[black]));

    // Side to move
    if (fenActiveColor[0] != 'w') {
        toggleSideToMove();
        mKey ^= mZobrist.getSTMKey();
    }

    // Castling rights
    for (char c : fenCastlingRights) {
        switch (c) {
        case 'k': mStateHist.back() |= uint32_t(1) << 2; break;
        case 'q': mStateHist.back() |= uint32_t(1) << 4; break;
        case 'K': mStateHist.back() |= uint32_t(1) << 1; break;
        case 'Q': mStateHist.back() |= uint32_t(1) << 3; break;
        }
    }
    mKey ^= mZobrist.getCastleKey(getCastles());

    // En passant square
    if (fenEpSquare != "-") {
        setEpSquare(fenEpSquare[0] - 'a' + 8 * (1 - getSideToMove()));
        mKey ^= mZobrist.getEPKey(getEpSquare() % 8); 
    }

    // Halfmove clock
    mStateHist.back() |= (fenHalfmoveClock & 0x7f) << 25;
}

Board::Board(const Board &tOther) : 
    mBitboards{tOther.mBitboards}, 
    mKey(tOther.mKey) ,
    mZobrist{Zobrist::getInstance()}
{
    if(tOther.mStateHist.size())mStateHist.emplace_back(tOther.mStateHist.back());
}

Board &Board::operator=(const Board &tOther)
{
    if(this != &tOther){
        mBitboards = tOther.mBitboards;
        mKey = tOther.mKey;
        if(tOther.mStateHist.size())mStateHist.emplace_back(tOther.mStateHist.back());
    }
    return *this;
}


bool Board::operator==(const Board &tOther) const
{
    static constexpr uint32_t mask = uint32_t(0x7f) << 25 | uint32_t(0x7) << 10;
    return (mBitboards == tOther.mBitboards) && ((mStateHist.back() & ~mask) == (tOther.mStateHist.back() & ~mask));
}

bool Board::operator!=(const Board &tOther) const
{
    static constexpr uint32_t mask = uint32_t(0x7f) << 25 | uint32_t(0x7) << 10;
    return (mBitboards != tOther.mBitboards) || ((mStateHist.back() & ~mask) != (tOther.mStateHist.back() & ~mask));
}

std::string Board::asString() const {
    static constexpr std::array<char, 6>  wPieces = {'P', 'N', 'B', 'R', 'Q', 'K'};
    static constexpr std::array<char, 6>  bPieces = {'p', 'n', 'b', 'r', 'q', 'k'};
    static constexpr std::array<const char*, 16> castleRights = {
        "-", "K", "k", "Kk", "Q", "KQ", "Qk", "KQk",
        "q", "Kq", "kq", "Kkq", "Qq", "KQq", "Qkq", "KQkq"
    };
    static constexpr std::array<const char*, 16> EPmap  = {
        "a3","b3","c3","d3","e3","f3","g3","h3",
        "a6","b6","c6","d6","e6","f6","g6","h6"
    };

    const uint64_t wBitBoard = getBitboard(white);
    const uint64_t bBitBoard = getBitboard(black);

    std::ostringstream out;

    auto flush = [&](int& count) {
        if (count > 0) {
            out << count;
            count = 0;
        }
    };

    int emptyAdj = 0;
    for (int rank = 7; rank >= 0; rank --){
        for (int file = 0; file <= 7; file ++){
            int square = 8*rank + file;
            uint64_t bitMask = uint64_t(1) << square;
            if      (bitMask & wBitBoard){
            flush(emptyAdj);
            out << wPieces[searchPiece(square) - pawn];
            }
            else if (bitMask & bBitBoard){
                flush(emptyAdj);
                out << bPieces[searchPiece(square) - pawn];
            }
            else 
                emptyAdj += 1;
        }

        flush(emptyAdj);
        out << (rank == 0 ? ' ' : '/');
    }

    out  << (getSideToMove() == white ? 'w' : 'b') << ' '
        << castleRights[getCastles()] << ' '
        << (getEpState() == true ? EPmap[getEpSquare() - a4] : "-") << ' '
        << getHMC() << ' ' << getFMC();    

    return out.str();
}

void Board::makeMove(const Move &tMove)
{
    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);

    int piece, captured;
    int from = tMove.from();
    int to = tMove.to();
    int stm = getSideToMove();

    // ALWAYS removes captured piece, en-passant state and en-passant square
    // removes castling depending on the move type
    static constexpr std::array<uint32_t, 64> stateMask= []{
        std::array<uint32_t, 64> a {};

        auto fillArray = [&](int square){
            uint32_t epMask = ~(0x1fu << 5);
            uint32_t capturedMask = ~(0x7u << 10);

            a[square] = UINT32_MAX & epMask & capturedMask;

            if      (square == a1) a[square] &= ~(1U << 3);
            else if (square == h1) a[square] &= ~(1U << 1);
            else if (square == e1) a[square] &= ~(1U << 1) & ~(1U << 3);
            else if (square == a8) a[square] &= ~(1U << 4);
            else if (square == h8) a[square] &= ~(1U << 2);
            else if (square == e8) a[square] &= ~(1U << 4) & ~(1U << 2);
        };

        for (int square = a1; square <= h8; square ++) fillArray(square);
        return a;
    }();

    mStateHist.emplace_back(mStateHist.back());
    mStateHist.back() &= stateMask[from] & stateMask[to];
    incrementHMC();
    static constexpr std::array<int, 2> KCoffset = {a1, a8}; //adds eight rank values only if stm is black
    static constexpr std::array<int, 2> QCoffset = {a1, a8}; //adds eight rank values only if stm is black
    static constexpr std::array<int, 2> EPoffset = {-8, 8};

    switch (tMove.flag()){
    case quiet:
        piece = searchPiece(from);
        movePiece(stm, piece, from, to);    
        if (piece == pawn) resetHMC();
        else if (piece == king) setKingSquare(stm, to);
        break;
    case doublePush:
        movePiece(stm, pawn, from, to);
        setEpSquare(to);
        resetHMC();
        break;
    case kingCastle:
        movePiece(stm, king, from, to);
        movePiece(stm, rook, h1 + KCoffset[stm], f1 + KCoffset[stm]); 
        setKingSquare(stm, to);
        break;
    case queenCastle:
        movePiece(stm, king, from, to);
        movePiece(stm, rook, a1 + QCoffset[stm], d1 + QCoffset[stm]); 
        setKingSquare(stm, to);
        break;
    case capture:
        piece = searchPiece(from);
        captured = searchPiece(to);
        movePiece(stm, piece, from, to);
        capturePiece(stm, captured, to);

        if (piece == king) setKingSquare(stm, tMove.to());
        setCaptured(captured);
        resetHMC();
        break;
    case enPassant:
        movePiece(stm, pawn, from, to);
        capturePiece(stm, pawn, to + EPoffset[stm]);
        setCaptured(pawn);
        resetHMC();
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        captured = searchPiece(to);
        capturePiece(stm, captured, to);
        setCaptured(captured);
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        promotePiece(stm, tMove.promoPiece(), from, to);
        resetHMC();
        break;
    }

    toggleSideToMove();

    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);
}

void Board::undoMove(const Move &tMove)
{
    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);

    toggleSideToMove();
    int move_from = tMove.from();
    int move_to = tMove.to();
    int stm = getSideToMove();
    static constexpr std::array<int, 2> KCoffset = {a1, a8}; //adds eight rank values only if stm is black
    static constexpr std::array<int, 2> QCoffset = {a1, a8}; //adds eight rank values only if stm is black
    static constexpr std::array<int, 2> EPoffset = {-8, 8};

    
    switch (tMove.flag()){
    case quiet:
        movePiece(stm, searchPiece(move_to), move_from, move_to);
        break;
    case doublePush:
        movePiece(stm, pawn, move_from, move_to);
        break;
    case kingCastle:
        movePiece(stm, king, move_from, move_to);
        movePiece(stm, rook, h1 + KCoffset[stm], f1 + KCoffset[stm]); 
        break;
    case queenCastle:
        movePiece(stm, king, move_from, move_to);
        movePiece(stm, rook, a1 + QCoffset[stm], d1 + QCoffset[stm]); 
        break;
    case capture:
        movePiece(stm, searchPiece(move_to), move_from, move_to);
        capturePiece(stm, getCaptured(), move_to);
        break;
    case enPassant:
        movePiece(stm, pawn, move_from, move_to);
        capturePiece(stm, pawn, move_to + EPoffset[stm]);
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        capturePiece(stm, getCaptured(), move_to);
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        promotePiece(stm, tMove.promoPiece(), move_from, move_to);
        break;
    }

    mStateHist.pop_back();

    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);
}

int Board::searchPiece(int tSquare) const{
    uint64_t mask = 1ULL << tSquare;
    for (int piece = pawn; piece <= king; piece ++) 
        if (mBitboards[piece] & mask) return piece;
    return 0;
}

void Board::movePiece(int tSTM, int tPiece, int tFrom, int tTo) {
    uint64_t mask = 1ULL << tFrom | 1ULL << tTo;
    mBitboards[tPiece] ^= mask;
    mBitboards[tSTM]   ^= mask;
    mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tFrom);
    mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tTo);
}

void Board::capturePiece(int tSTM, int tPiece, int tSquare)
{
    uint64_t mask = 1ULL << tSquare;
    mBitboards[tPiece] ^= mask;
    mBitboards[1-tSTM] ^= mask;
    mKey ^= mZobrist.getPieceKey(1-tSTM, tPiece, tSquare);
}

void Board::promotePiece(int tSTM, int tPiece, int tFrom, int tTo)
{
    uint64_t maskTo = 1ULL << tTo;
    uint64_t maskFrom = 1ULL << tFrom;
    mBitboards[pawn]  ^= maskFrom;
    mBitboards[tPiece] ^= maskTo;
    mBitboards[tSTM]   ^= maskFrom | maskTo;
    mKey ^= mZobrist.getPieceKey(tSTM, pawn, tFrom);
    mKey ^= mZobrist.getPieceKey(tSTM, tPiece, tTo);
}
