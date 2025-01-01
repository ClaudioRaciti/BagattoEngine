#include "HashTable.hpp"

void HashTable::insert(Board &t_key, const int16_t &t_score, const int &t_depth, const int &t_nodeType)
{
    int index = t_key.getHash() % m_size;
    m_table[index] = Entry{t_key, TTValue{t_score, t_depth, t_nodeType}};
}

int16_t HashTable::getScore(const Board &t_key) const
{
    int index = t_key.getHash() % m_size;
    return m_table[index]->value.score();
}

int HashTable::getDepth(const Board &t_key) const
{
    int index = t_key.getHash() % m_size;
    return m_table[index]->value.depth();
}

int HashTable::getNodeType(const Board &t_key) const
{
    int index = t_key.getHash() % m_size;
    return m_table[index]->value.nodeType();
}

bool HashTable::contains(const Board &t_key) const
{
    int index = t_key.getHash() % m_size;
    return m_table[index]->key == t_key;
}
