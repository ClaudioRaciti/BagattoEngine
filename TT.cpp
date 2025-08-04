#include "TT.hpp"
#include <cassert>
#include <cstdint>
#include <tuple>

TT::TT(int tMBSize)
{
    mSize = tMBSize * 1024 * 1024 / sizeof(TTEntry);
    mTable = new TTEntry[mSize];
}

TT::~TT()
{
    delete[] mTable;
}

void TT::resize(int tMBSize)
{
    mSize = tMBSize * 1024 * 1024 / sizeof(TTEntry);
    delete[] mTable;
    mTable = new TTEntry[mSize];
}

void TT::insert(TTEntry tEntry){
    size_t index = tEntry.key % mSize;
    mTable[index] = tEntry;
}

std::tuple<bool, TTEntry> TT::probe(uint64_t tKey){
    size_t index = tKey % mSize;
    TTEntry& entry = mTable[index];

    return {entry.key == tKey, entry};
}