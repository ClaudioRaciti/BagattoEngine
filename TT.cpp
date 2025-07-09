#include "TT.hpp"

#include <cassert>

TT::TT(int sizeMB)
{
    m_tableSize = sizeMB * 1024 * 1024 / sizeof(Entry);
    m_table = new Entry[m_tableSize];
    std::fill(m_table, m_table + m_tableSize, Entry{});
}

TT::~TT()
{
    delete[] m_table;
}

void TT::insert(uint64_t tKey, int16_t tScore, int tDepth, int tNodeType, Move tHashMove)
{
    size_t index = tKey % m_tableSize;
    m_table[index] = Entry{tKey, TTValue(tScore, tDepth, tNodeType), tHashMove};
}

void TT::resize(int sizeMB)
{
    m_tableSize = sizeMB * 1024 * 1024 / sizeof(Entry);
    delete[] m_table;
    m_table = new Entry[m_tableSize];
    std::cout << "Successfully allocated " << sizeMB << "MB \t" << m_tableSize << " objects" << std::endl;
}
