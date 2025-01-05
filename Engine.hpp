#pragma once

#include <vector>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "HashTable.hpp"
#include "LookupTables.hpp"

class Engine
{
private:
    HashTable m_TTable;
    const LookupTables &m_lookup;
    Board m_board;
    MoveGenerator m_generator;
    int m_materialCount;
public:
    Engine(): m_TTable{1 << 22}, m_lookup{LookupTables::getInstance()}{}
    ~Engine() = default;

    int16_t getEval(std::string t_position, int t_depth);
    int16_t debugQuiescence(std::string t_position);
private:
    int16_t iterativeDeepening(int t_depth, int t_maxDepth, int16_t alpha, int16_t beta);
    int16_t alphaBeta(int t_depth, int16_t alpha, int16_t beta, std::vector<Move> &t_PV);
    int16_t quiescence(int16_t t_alpha, int16_t t_beta);
    int gamePhase();
    bool isIllegal(); // opponent side is in check but its not his turn
    bool isCheck();   // opponent side gives check and its your turn
    bool isSqAttacked(int t_square, int t_attackingSide);
};
