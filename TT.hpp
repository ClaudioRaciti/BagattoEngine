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
public:
    // Constructor
    explicit TT(int sizeMB);
    ~TT();
    
    /**
     * @brief Drops the current table and allocates a new one of given size
     * 
     * @param sizeMB New size of the hash table in MB
     */
    void resize(int sizeMB);

    /**
     * @brief Inserts an entry
     * 
     * @param tEntry Position information as specified in the designated struct
     */
    void insert(TTEntry tEntry);

    /**
     * @brief Probes the hash table and returns an entry
     * 
     * @param tKey Zobrist hash key
     * @return std::tuple<bool, TTEntry> composed of the truth value for probe hit and a TTEntry
     */
    std::tuple<bool, TTEntry> probe(uint64_t tKey);

private:
    size_t mSize;    // Fixed size of the hash table
    TTEntry* mTable; // Fixed-size vector of optional entries
};