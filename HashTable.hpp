#pragma once

#include <array>

#include "Board.hpp"
#include "TTValue.hpp"

#include <optional>

class HashTable {
private:
    struct Entry {
        Board key;
        TTValue value;
    };

    int m_size;                             // Fixed size of the hash table
    std::vector<std::optional<Entry>> m_table; // Fixed-size vector of optional entries

public:
    // Constructor
    explicit HashTable(int tableSize) : m_size(tableSize), m_table(tableSize) {}

    // Insert a key-value pair (overwriting on collision)
    void insert(Board &t_key, const int16_t &t_score, const int &t_depth, const int &t_nodeType);

    // Retrieve a value by key
    int16_t getScore(const Board& t_key) const;

    int getDepth(const Board& t_key) const;

    int getNodeType(const Board& t_key) const;

    // Check if key has been inserted
    bool contains(const Board& t_key) const;
};