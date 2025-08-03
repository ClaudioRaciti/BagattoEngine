#pragma once

#include "TTValue.hpp"
#include "Move.hpp"

class TT {
private:
    struct Entry {
        uint64_t key = 0ULL;
        TTValue value;
        Move hashMove;
    };

    size_t mSize;                             // Fixed size of the hash table
    Entry* mTable; // Fixed-size vector of optional entries

public:
    // Constructor
    explicit TT(int sizeMB);
    ~TT();

    // Insert a key-value pair (overwriting on collision)
    void insert(uint64_t tKey, int16_t tScore, int tDepth, int tNodeType, Move tHashMove);
    void resize(int sizeMB);

    // Retrieve a value by key
    inline TTValue getValue(uint64_t tKey) const {return mTable[tKey % mSize].value;}

    inline int16_t getScore(uint64_t tKey) const{return mTable[tKey % mSize].value.score();}

    inline Move getMove(uint64_t tKey) const {return mTable[tKey % mSize].hashMove;}

    inline int getDepth(uint64_t tKey) const {return mTable[tKey % mSize].value.depth();}

    inline int getNodeType(uint64_t tKey) const {return mTable[tKey % mSize].value.nodeType();}

    // Check if key has been inserted
    inline bool contains(uint64_t tKey) const {return mTable[tKey % mSize].key == tKey;}
};