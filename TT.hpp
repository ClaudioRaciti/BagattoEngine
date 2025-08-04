#pragma once

#include "Move.hpp"
#include <cstdint>
#include <tuple>

struct TTEntry{
    uint64_t key;
    int16_t score;
    uint8_t depht = 0;
    uint8_t nodeType;
    Move hashMove;

    TTEntry() = default;
    TTEntry(uint64_t tKey, int16_t tScore, uint8_t tDepth, uint8_t tNodeType, Move tMove) :
        key{tKey}, score(tScore), depht(tDepth), nodeType(tNodeType), hashMove(tMove){}
};

class TT {
private:

    size_t mSize;                             // Fixed size of the hash table
    TTEntry* mTable;                            // Fixed-size vector of optional entries

public:
    // Constructor
    explicit TT(int sizeMB);
    ~TT();

    // Insert a key-value pair (overwriting on collision)
    void resize(int sizeMB);
    void insert(TTEntry tEntry);
    std::tuple<bool, TTEntry> probe(uint64_t tKey);
};