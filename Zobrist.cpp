#include "Zobrist.hpp"
#include <unordered_set>
#include <cassert>

Zobrist* Zobrist::m_instance = nullptr;

Zobrist::Zobrist(): 
    rng{std::mt19937_64(5829046653945461000ULL)}, 
    device{std::uniform_int_distribution<uint64_t>(0, UINT64_MAX)}{
    initPieces();
    initCastle();
    initEP();
    initSideToMove();
}

const Zobrist& Zobrist::getInstance(){
    if (m_instance == nullptr) m_instance = new Zobrist();
    return *m_instance;
}

void Zobrist::initPieces(){
    for (uint64_t& val : m_pieceKey) val = device(rng);
}

void Zobrist::initCastle(){
    for (uint64_t& val : m_castleKey) val = device(rng);
}

void Zobrist::initEP(){
    for (uint64_t& val : m_EPKey) val = device(rng);
}

void Zobrist::initSideToMove(){
    m_STMKey = device(rng);
}