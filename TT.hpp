#pragma once

#include <array>

#include "Board.hpp"
#include "TTValue.hpp"
#include "Move.hpp"

class TT {
private:
    struct Entry {
        uint64_t key;
        TTValue value;
        Move hashMove;
    };

    size_t m_tableSize;                             // Fixed size of the hash table
    Entry* m_table; // Fixed-size vector of optional entries

public:
    // Constructor
    explicit TT(int sizeMB);
    ~TT();

    // Insert a key-value pair (overwriting on collision)
    void insert(Board &t_key, const int16_t&t_score, const int &t_depth, const int &t_nodeType, const Move &t_hashMove);
    void resize(int sizeMB);

    // Retrieve a value by key
    int16_t getScore(const Board& t_key) const;

    int getDepth(const Board& t_key) const;

    int getNodeType(const Board& t_key) const;

    // Check if key has been inserted
    bool contains(const Board& t_key) const;
    bool contains(const Board& t_key, int16_t &t_score, int &t_depth, int &t_nodeType, Move &t_hashMove);
};