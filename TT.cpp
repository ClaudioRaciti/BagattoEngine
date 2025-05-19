#include "TT.hpp"

TT::TT(int sizeMB)
{
    m_tableSize = sizeMB * 1024 * 1024 / sizeof(Entry);
    m_table = new Entry[m_tableSize];
}

TT::~TT()
{
    delete[] m_table;
}

void TT::insert(Board &t_key, const int16_t &t_score, const int &t_depth, const int &t_nodeType, const Move &t_hashMove)
{
    uint64_t zobristKey = t_key.getHash();
    size_t index = zobristKey % m_tableSize;
    m_table[index] = Entry{(zobristKey), TTValue(t_score, t_depth, t_nodeType), t_hashMove};
}

void TT::resize(int sizeMB)
{
    m_tableSize = sizeMB * 1024 * 1024 / sizeof(Entry);
    delete[] m_table;
    m_table = new Entry[m_tableSize];
    std::cout << "Number of elements is " << m_tableSize << std::endl;
}

int16_t TT::getScore(const Board &t_key) const
{
    size_t index = t_key.getHash() % m_tableSize;
    return (m_table[index].value.score());
}

int TT::getDepth(const Board &t_key) const
{
    size_t index = t_key.getHash() % m_tableSize;
    return m_table[index].value.depth();
}

int TT::getNodeType(const Board &t_key) const
{
    size_t index = t_key.getHash() % m_tableSize;
    return m_table[index].value.nodeType();
}

bool TT::contains(const Board &t_key) const
{
    uint64_t zobristKey = t_key.getHash();
    size_t index = zobristKey % m_tableSize;
    return m_table[index].key == (zobristKey);
}

bool TT::contains(const Board &t_key, int16_t &t_score, int &t_depth, int &t_nodeType, Move &t_hashMove)
{
    uint64_t zobristKey = t_key.getHash();
    size_t index = zobristKey % m_tableSize;
    if (m_table[index].key == zobristKey){
        t_score = m_table[index].value.score();
        t_depth = m_table[index].value.depth();
        t_nodeType = m_table[index].value.nodeType();
        t_hashMove = m_table[index].hashMove;
        return true;
    }
    return false;
}
