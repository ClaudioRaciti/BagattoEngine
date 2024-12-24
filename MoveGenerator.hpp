#pragma once

#include <vector>

#include "Move.hpp"
#include "LookupTables.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator();
    
    std::vector<Move> generateQuietMoves (const Board&);
    std::vector<Move> generateCaptures   (const Board&);
private:
    const LookupTables& m_lookup;
};