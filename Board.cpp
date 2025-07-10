#include "Board.hpp"
#include "utils.hpp"
#include <cassert>
#include <bitset>
#include <random>

Board::Board(std::string tFEN):
    mZobrist{Zobrist::getInstance()}{
    std::vector<std::string> dataFields = split(tFEN, ' ');
    std::string boardField = dataFields[0];
    std::string sideToMove = dataFields[1];
    std::string castleRights = dataFields[2];
    std::string epSquare = dataFields[3];
    std::string halfmoveClock = dataFields.size() < 5 ? "0" : dataFields[4];

    mStateHist.emplace_back(uint32_t(0x0));

    // Set bitboards to zero
    for(int i = 0; i < 8; i ++) mBitboards[i] = 0ULL;

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
                mBitboards[pawn]  |= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, pawn, squareNumber);
                break;
            case 'P': 
                mBitboards[pawn]  |= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, pawn, squareNumber);
                break;
            case 'n': 
                mBitboards[knight]|= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, knight, squareNumber);
                break;
            case 'N': 
                mBitboards[knight]|= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, knight, squareNumber);
                break;
            case 'b': 
                mBitboards[bishop]|= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, bishop, squareNumber);
                break;
            case 'B': 
                mBitboards[bishop]|= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, bishop, squareNumber);
                break;
            case 'r': 
                mBitboards[rook]  |= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, rook, squareNumber);
                break;
            case 'R': 
                mBitboards[rook]  |= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, rook, squareNumber);
                break;
            case 'q': 
                mBitboards[queen] |= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, queen, squareNumber);
                break;
            case 'Q': 
                mBitboards[queen] |= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, queen, squareNumber);
                break;
            case 'k': 
                mBitboards[king]  |= uint64_t(1) << squareNumber; 
                mBitboards[black] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(black, king, squareNumber);
                break;
            case 'K': 
                mBitboards[king]  |= uint64_t(1) << squareNumber; 
                mBitboards[white] |= uint64_t(1) << squareNumber; 
                mKey ^= mZobrist.getPieceKey(white, king, squareNumber);
                break;
            }
        }
    }

    setKingSquare(white, bitScanForward(mBitboards[king] & mBitboards[white]));
    setKingSquare(black, bitScanForward(mBitboards[king] & mBitboards[black]));

    // Set side to move
    if (tolower(sideToMove[0]) != 'w') {
        toggleSideToMove();
        mKey ^= mZobrist.getSTMKey();
    }

    // Set castling rights
    for (char character : castleRights) switch (character)
    {
    case 'k': mStateHist.back() |= uint32_t(1) << 2; break;
    case 'q': mStateHist.back() |= uint32_t(1) << 4; break;
    case 'K': mStateHist.back() |= uint32_t(1) << 1; break;
    case 'Q': mStateHist.back() |= uint32_t(1) << 3; break;
    }
    mKey ^= mZobrist.getCastleKey(getCastles());

    // Set ep square
    if (epSquare.compare("-") != 0) {
        setEpSquare(atoi(epSquare.c_str()));
        mKey ^= mZobrist.getEPKey(getEpSquare()%8); 
    }

    // Set HMC
    for (int i = 0; i < atoi(halfmoveClock.c_str()); i ++) incrementHMC();

    // Set material count
    mMaterialCount = 0;
    static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    for (int piece = pawn; piece < king; piece ++) mMaterialCount += popCount(mBitboards[piece]) * pieceVal[piece];
}

Board::Board(const Board &tOther) : 
    mBitboards{tOther.mBitboards}, 
    mKey(tOther.mKey) ,
    mMaterialCount{tOther.mMaterialCount},
    mZobrist{Zobrist::getInstance()}
{
    if(tOther.mStateHist.size())mStateHist.emplace_back(tOther.mStateHist.back());
}

Board &Board::operator=(const Board &tOther)
{
    if(this != &tOther){
        mBitboards = tOther.mBitboards;
        mMaterialCount = tOther.mMaterialCount;
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

uint64_t Board::getHash() const
{
    return mKey;
}

void Board::makeMove(const Move &tMove)
{
    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);

    int piece, captured;
    int from = tMove.from();
    int to = tMove.to();
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
    mStateHist.emplace_back(mStateHist.back());
    mStateHist.back() &= stateMask[from] & stateMask[to];
    incrementHMC();

    switch (tMove.flag()){
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

        if (piece == king) setKingSquare(stm, tMove.to());
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
        promotePiece(stm, tMove.promoPiece(), from, to);
        decreaseMaterialCount(pawn);
        increaseMaterialCount(tMove.promoPiece());
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

    
    switch (tMove.flag()){
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
        promotePiece(stm, tMove.promoPiece(), move_from, move_to);
        increaseMaterialCount(pawn);
        decreaseMaterialCount(tMove.promoPiece());
        break;
    }

    mStateHist.pop_back();

    mKey ^= mZobrist.getCastleKey(getCastles());
    if (getEpState()) mKey ^= mZobrist.getEPKey(getEpSquare()%8);
}

int Board::searchMoved(int tSquare) const
{
    int stm = getSideToMove();
    uint64_t mask = 1ULL << tSquare & mBitboards[stm];
    for (int piece = pawn; piece <= king; piece ++) 
        if (mBitboards[piece] & mask) return piece;
    throw std::runtime_error("moved piece not found");
}

int Board::searchCaptured(int tSquare) const
{
    int stm = getSideToMove();
    uint64_t mask = 1ULL << tSquare & mBitboards[1-stm];
    for (int piece = queen; piece >= pawn; piece--) 
        if (mBitboards[piece] & mask) return piece;
    throw std::runtime_error("captured piece not found");
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
