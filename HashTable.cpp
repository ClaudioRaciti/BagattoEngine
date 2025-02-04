#include "HashTable.hpp"

void HashTable::insert(Board &t_key, const int16_t &t_score, const int &t_depth, const int &t_nodeType, const Move &t_hashMove)
{
    int index = t_key.getHash() % m_size;
    m_table[index] = Entry{t_key, TTValue(t_score, t_depth, t_nodeType), t_hashMove};
}

int16_t HashTable::getScore(const Board &t_key) const
{
    int index = t_key.getHash() % m_size;
    return (m_table[index]->value.score());
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

bool HashTable::contains(const Board &t_key, int16_t &t_score, int &t_depth, int &t_nodeType, Move &t_hashMove)
{
    int index = t_key.getHash() % m_size;
    if (m_table[index]->key == t_key){
        t_score = m_table[index]->value.score();
        t_depth = m_table[index]->value.depth();
        t_nodeType = m_table[index]->value.nodeType();
        t_hashMove = m_table[index]->hashMove;
        return true;
    }
    return false;
}
