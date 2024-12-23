#pragma once

#include <vector>

#include "Move.hpp"
#include "LookupTables.hpp"
#include "BitBoards.hpp"

class MoveGenerator{
public:
    MoveGenerator();
    
    std::vector<Move> generateQuietMoves(const BitBoards& t_bitBoards);
    std::vector<Move> generateCaptures(const BitBoards& t_bitBoards);
private:
    const LookupTables& m_lookup;
};