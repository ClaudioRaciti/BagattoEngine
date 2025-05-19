#pragma once

#include <vector>

#include "Move.hpp"
#include "LookupTables.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator() : m_lookup{LookupTables::getInstance()} {};
    std::vector<Move> generateMoves(const Board&) const;
    std::vector<Move> generateQuiets(const Board&) const;
    void generateQuiets(const Board&, std::vector<Move> &) const;
    std::vector<Move> generateCaptures(const Board&) const;
    void generateCaptures(const Board&, std::vector<Move> &) const;
    std::vector<Move> evadeCheck(const Board&) const;
    void evadeCheck(const Board&, std::vector<Move> &) const;
    bool isSquareAttacked(const Board &t_board, int t_square, int t_attackingSide) const;
private:
    void generatePieceQuiets(int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generatePawnsQuiets(std::vector<Move> &t_moveList, const Board &t_board) const;
    void generateCatstle(std::vector<Move> &t_moveList, const Board &t_board) const; 

    void generatePieceCaptures(int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generatePawnsCaptures(std::vector<Move> &t_moveList, const Board &t_board) const;

    void captureAttacker(int t_attacker, int t_attackerSq, std::vector<Move> &t_moveList, const Board &t_board) const;
    void blockAttacker(uint64_t t_blockableSquares, std::vector<Move> &t_moveList, const Board &t_board) const;

    bool isCheck(const Board &t_board) const;
    uint64_t getAttacksTo(const Board &t_board, int t_square, int t_attackingSide) const;
private:
    const LookupTables& m_lookup;
};