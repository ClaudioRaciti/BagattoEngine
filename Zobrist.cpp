#include "Zobrist.hpp"
#include <cassert>
#include <cstdint>

Zobrist* Zobrist::mInstance = nullptr;

Zobrist::Zobrist(): 
    mRng{std::mt19937_64(5829046653945461000ULL)}{
    initPieces();
    initCastle();
    initEP();
    initSideToMove();
}

const Zobrist& Zobrist::getInstance(){
    if (mInstance == nullptr) mInstance = new Zobrist();
    return *mInstance;
}

void Zobrist::initPieces(){
    for (uint64_t& val : mPieceKeys) val = mRng();
}

void Zobrist::initCastle(){
    for (uint64_t& val : mCastleKeys) val = mRng();
}

void Zobrist::initEP(){
    for (uint64_t& val : mEPKeys) val = mRng();
}

void Zobrist::initSideToMove(){
    mSTMKey = mRng();
}