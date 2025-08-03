#include "Zobrist.hpp"
#include <cassert>

Zobrist* Zobrist::mInstance = nullptr;

Zobrist::Zobrist(): 
    mRng{std::mt19937_64(5829046653945461000ULL)}, 
    mDevice{std::uniform_int_distribution<uint64_t>(0, UINT64_MAX)}{
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
    for (uint64_t& val : mPieceKeys) val = mDevice(mRng);
}

void Zobrist::initCastle(){
    for (uint64_t& val : mCastleKeys) val = mDevice(mRng);
}

void Zobrist::initEP(){
    for (uint64_t& val : mEPKeys) val = mDevice(mRng);
}

void Zobrist::initSideToMove(){
    mSTMKey = mDevice(mRng);
}