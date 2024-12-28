#pragma once

#include <vector>

#include "Move.hpp"
#include "LookupTables.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator() : m_lookup{LookupTables::getInstance()} {};
    std::vector<Move> generateMoves(const Board&);
    std::vector<Move> generateQuiets(const Board&);
    void generateQuiets(const Board&, std::vector<Move> &);
    std::vector<Move> generateCaptures(const Board&);
    void generateCaptures(const Board&, std::vector<Move> &);
private:
    void generatePieceQuiets(int t_piece, std::vector<Move> &t_moveList, const Board &t_board);
    void generatePawnsQuiets(std::vector<Move> &t_moveList, const Board &t_board);
    void generateCatstle(std::vector<Move> &t_moveList, const Board &t_board); 

    void generatePieceCaptures(int t_piece, std::vector<Move> &t_moveList, const Board &t_board);
    void generatePawnsCaptures(std::vector<Move> &t_moveList, const Board &t_board);

    bool isSquareAttacked(const Board &t_board, int t_square, int t_attackingSide);
private:
    const LookupTables& m_lookup;
};