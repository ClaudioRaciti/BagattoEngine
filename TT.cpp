#include "TT.hpp"

#include <cassert>

TT::TT(int tMBSize)
{
    mSize = tMBSize * 1024 * 1024 / sizeof(Entry);
    mTable = new Entry[mSize];
    std::fill(mTable, mTable + mSize, Entry{});
}

TT::~TT()
{
    delete[] mTable;
}

void TT::insert(uint64_t tKey, int16_t tScore, int tDepth, int tNodeType, Move tHashMove)
{
    size_t index = tKey % mSize;
    mTable[index] = Entry{tKey, TTValue(tScore, tDepth, tNodeType), tHashMove};
}

void TT::resize(int tMBSize)
{
    mSize = tMBSize * 1024 * 1024 / sizeof(Entry);
    delete[] mTable;
    mTable = new Entry[mSize];
    std::cout << "Successfully allocated " << tMBSize << "MB \t" << mSize << " objects" << std::endl;
}
