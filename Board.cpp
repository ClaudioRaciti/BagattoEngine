#include "Board.hpp"
#include "utils.hpp"
#include <cassert>
#include <bitset>
#include <random>

Board::Board(std::string t_FEN):
    m_zobrist{Zobrist::getInstance()}{
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
            case 'p': 
                m_bitboard[pawn]  |= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, pawn, squareNumber);
                break;
            case 'P': 
                m_bitboard[pawn]  |= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, pawn, squareNumber);
                break;
            case 'n': 
                m_bitboard[knight]|= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, knight, squareNumber);
                break;
            case 'N': 
                m_bitboard[knight]|= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, knight, squareNumber);
                break;
            case 'b': 
                m_bitboard[bishop]|= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, bishop, squareNumber);
                break;
            case 'B': 
                m_bitboard[bishop]|= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, bishop, squareNumber);
                break;
            case 'r': 
                m_bitboard[rook]  |= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, rook, squareNumber);
                break;
            case 'R': 
                m_bitboard[rook]  |= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, rook, squareNumber);
                break;
            case 'q': 
                m_bitboard[queen] |= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, queen, squareNumber);
                break;
            case 'Q': 
                m_bitboard[queen] |= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, queen, squareNumber);
                break;
            case 'k': 
                m_bitboard[king]  |= uint64_t(1) << squareNumber; 
                m_bitboard[black] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(black, king, squareNumber);
                break;
            case 'K': 
                m_bitboard[king]  |= uint64_t(1) << squareNumber; 
                m_bitboard[white] |= uint64_t(1) << squareNumber; 
                m_zobristKey ^= m_zobrist.getPieceKey(white, king, squareNumber);
                break;
            }
        }
    }

    setKingSquare(white, bitScanForward(m_bitboard[king] & m_bitboard[white]));
    setKingSquare(black, bitScanForward(m_bitboard[king] & m_bitboard[black]));

    // Set side to move
    if (tolower(sideToMove[0]) != 'w') {
        toggleSideToMove();
        m_zobristKey ^= m_zobrist.getSTMKey();
    }

    // Set castling rights
    for (char character : castleRights) switch (character)
    {
    case 'k': m_stateHist.back() |= uint32_t(1) << 2; break;
    case 'q': m_stateHist.back() |= uint32_t(1) << 4; break;
    case 'K': m_stateHist.back() |= uint32_t(1) << 1; break;
    case 'Q': m_stateHist.back() |= uint32_t(1) << 3; break;
    }
    m_zobristKey ^= m_zobrist.getCastleKey(getCastles());

    // Set ep square
    if (epSquare.compare("-") != 0) {
        setEpSquare(atoi(epSquare.c_str()));
        m_zobristKey ^= m_zobrist.getEPKey(getEpSquare()%8); 
    }

    // Set HMC
    for (int i = 0; i < atoi(halfmoveClock.c_str()); i ++) incrementHMC();

    // Set material count
    m_materialCount = 0;
    static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    for (int piece = pawn; piece < king; piece ++) m_materialCount += popCount(m_bitboard[piece]) * pieceVal[piece];
}

Board::Board(const Board &t_other) : 
    m_bitboard{t_other.m_bitboard}, 
    m_zobristKey(t_other.m_zobristKey) ,
    m_materialCount{t_other.m_materialCount},
    m_zobrist{Zobrist::getInstance()}
{
    if(t_other.m_stateHist.size())m_stateHist.emplace_back(t_other.m_stateHist.back());
}

Board &Board::operator=(const Board &t_other)
{
    if(this != &t_other){
        m_bitboard = t_other.m_bitboard;
        m_materialCount = t_other.m_materialCount;
        m_zobristKey = t_other.m_zobristKey;
        if(t_other.m_stateHist.size())m_stateHist.emplace_back(t_other.m_stateHist.back());
    }
    return *this;
}


bool Board::operator==(const Board &t_other) const
{
    static constexpr uint32_t mask = uint32_t(0x7f) << 25 | uint32_t(0x7) << 10;
    return (m_bitboard == t_other.m_bitboard) && ((m_stateHist.back() & ~mask) == (t_other.m_stateHist.back() & ~mask));
}

bool Board::operator!=(const Board &t_other) const
{
    static constexpr uint32_t mask = uint32_t(0x7f) << 25 | uint32_t(0x7) << 10;
    return (m_bitboard != t_other.m_bitboard) || ((m_stateHist.back() & ~mask) != (t_other.m_stateHist.back() & ~mask));
}

uint64_t Board::getHash() const
{
    return m_zobristKey;
}

void Board::makeMove(const Move &t_move)
{
    m_zobristKey ^= m_zobrist.getCastleKey(getCastles());
    if (getEpState()) m_zobristKey ^= m_zobrist.getEPKey(getEpSquare()%8);

    int piece, captured;
    int from = t_move.from();
    int to = t_move.to();
    int stm = getSideToMove();

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
    m_stateHist.emplace_back(m_stateHist.back());
    m_stateHist.back() &= stateMask[from] & stateMask[to];
    incrementHMC();

    switch (t_move.flag()){
    case quiet:
        piece = searchMoved(from);
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
        static constexpr int KCoffset[2] = {a1, a8}; //adds eight rank values only if stm is black
        movePiece(stm, king, from, to);
        movePiece(stm, rook, h1 + KCoffset[stm], f1 + KCoffset[stm]); 
        setKingSquare(stm, to);
        break;
    case queenCastle:
        static constexpr int QCoffset[2] = {a1, a8}; //adds eight rank values only if stm is black
        movePiece(stm, king, from, to);
        movePiece(stm, rook, a1 + QCoffset[stm], d1 + QCoffset[stm]); 
        setKingSquare(stm, to);
        break;
    case capture:
        piece = searchMoved(from);
        captured = searchCaptured(to);
        movePiece(stm, piece, from, to);
        capturePiece(stm, captured, to);

        if (piece == king) setKingSquare(stm, t_move.to());
        decreaseMaterialCount(captured);
        setCaptured(captured);
        resetHMC();
        break;
    case enPassant:
        static constexpr int offset[2] = {-8, 8};
        movePiece(stm, pawn, from, to);
        capturePiece(stm, pawn, to + offset[stm]);
        decreaseMaterialCount(pawn);
        setCaptured(pawn);
        resetHMC();
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        captured = searchCaptured(to);
        capturePiece(stm, captured, to);
        decreaseMaterialCount(captured);
        setCaptured(captured);
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        promotePiece(stm, t_move.promoPiece(), from, to);
        decreaseMaterialCount(pawn);
        increaseMaterialCount(t_move.promoPiece());
        resetHMC();
        break;
    }

    toggleSideToMove();

    m_zobristKey ^= m_zobrist.getCastleKey(getCastles());
    if (getEpState()) m_zobristKey ^= m_zobrist.getEPKey(getEpSquare()%8);
}

void Board::undoMove(const Move &t_move)
{
    m_zobristKey ^= m_zobrist.getCastleKey(getCastles());
    if (getEpState()) m_zobristKey ^= m_zobrist.getEPKey(getEpSquare()%8);

    toggleSideToMove();
    int move_from = t_move.from();
    int move_to = t_move.to();
    int stm = getSideToMove();

    
    switch (t_move.flag()){
    case quiet:
        movePiece(stm, searchMoved(move_to), move_from, move_to);
        break;
    case doublePush:
        movePiece(stm, pawn, move_from, move_to);
        break;
    case kingCastle:
        static constexpr int KCoffset[2] = {a1, a8}; //adds eight rank values only if stm is black
        movePiece(stm, king, move_from, move_to);
        movePiece(stm, rook, h1 + KCoffset[stm], f1 + KCoffset[stm]); 
        break;
    case queenCastle:
        static constexpr int QCoffset[2] = {a1, a8}; //adds eight rank values only if stm is black
        movePiece(stm, king, move_from, move_to);
        movePiece(stm, rook, a1 + QCoffset[stm], d1 + QCoffset[stm]); 
        break;
    case capture:
        movePiece(stm, searchMoved(move_to), move_from, move_to);
        capturePiece(stm, getCaptured(), move_to);
        increaseMaterialCount(getCaptured());
        break;
    case enPassant:
        static constexpr int EPoffset[2] = {-8, 8}; //adds one rank offset depending on capturing side
        movePiece(stm, pawn, move_from, move_to);
        capturePiece(stm, pawn, move_to + EPoffset[stm]);
        increaseMaterialCount(pawn);
        break;
    case knightPromoCapture:
    case bishopPromoCapture:
    case rookPromoCapture:
    case queenPromoCapture:
        capturePiece(stm, getCaptured(), move_to);
        increaseMaterialCount(getCaptured());
        [[fallthrough]];
    case knightPromo:
    case bishopPromo:
    case rookPromo:
    case queenPromo:
        promotePiece(stm, t_move.promoPiece(), move_from, move_to);
        increaseMaterialCount(pawn);
        decreaseMaterialCount(t_move.promoPiece());
        break;
    }

    m_stateHist.pop_back();

    m_zobristKey ^= m_zobrist.getCastleKey(getCastles());
    if (getEpState()) m_zobristKey ^= m_zobrist.getEPKey(getEpSquare()%8);
}

int Board::searchMoved(int t_square) const
{
    int stm = getSideToMove();
    uint64_t mask = 1ULL << t_square & m_bitboard[stm];
    for (int piece = pawn; piece <= king; piece ++) 
        if (m_bitboard[piece] & mask) return piece;
    throw std::runtime_error("moved piece not found");
}

int Board::searchCaptured(int t_square) const
{
    int stm = getSideToMove();
    uint64_t mask = 1ULL << t_square & m_bitboard[1-stm];
    for (int piece = queen; piece >= pawn; piece--) 
        if (m_bitboard[piece] & mask) return piece;
    throw std::runtime_error("captured piece not found");
}

void Board::movePiece(int stm, int piece, int from, int to) {
    uint64_t mask = 1ULL << from | 1ULL << to;
    m_bitboard[piece] ^= mask;
    m_bitboard[stm]   ^= mask;
    m_zobristKey ^= m_zobrist.getPieceKey(stm, piece, from);
    m_zobristKey ^= m_zobrist.getPieceKey(stm, piece, to);
}

void Board::capturePiece(int stm, int piece, int sq)
{
    uint64_t mask = 1ULL << sq;
    m_bitboard[piece] ^= mask;
    m_bitboard[1-stm] ^= mask;
    m_zobristKey ^= m_zobrist.getPieceKey(1-stm, piece, sq);
}

void Board::promotePiece(int stm, int piece, int from, int to)
{
    uint64_t maskTo = 1ULL << to;
    uint64_t maskFrom = 1ULL << from;
    m_bitboard[pawn]  ^= maskFrom;
    m_bitboard[piece] ^= maskTo;
    m_bitboard[stm]   ^= maskFrom | maskTo;
    m_zobristKey ^= m_zobrist.getPieceKey(stm, pawn, from);
    m_zobristKey ^= m_zobrist.getPieceKey(stm, piece, to);
}
