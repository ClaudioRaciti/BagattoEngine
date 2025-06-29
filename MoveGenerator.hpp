#pragma once

#include <vector>

#include "Move.hpp"
#include "MagicBitboards.hpp"
#include "Board.hpp"

class MoveGenerator{
public:
    MoveGenerator() : m_lookup{MagicBitboards::getInstance()} {};
    inline void generateMoves (const Board& t_board, std::vector<Move>& t_list) const {generateCaptures(t_board, t_list); generateQuiets(t_board, t_list);}
    inline void generateQuiets(const Board& t_board, std::vector<Move>& t_list) const {generateQuiets(UINT64_MAX, t_board, t_list);}
    inline void generateCaptures(const Board& t_board, std::vector<Move>& t_list) const {generateCaptures(UINT64_MAX, t_board, t_list);}
    void evadeChecks(const Board&, std::vector<Move> &) const;
    bool isSquareAttacked(const Board &t_board, int t_square, int t_attackingSide) const;
private:
    void generateQuiets(uint64_t t_target, const Board& t_board, std::vector<Move>& t_list) const;
    void generateCaptures(uint64_t t_target, const Board& t_board, std::vector<Move>& t_list) const;

    void generatePieceQuiets(uint64_t t_target, int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generatePawnsQuiets(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generateCastle(std::vector<Move> &t_moveList, const Board &t_board) const; 

    void generatePieceCaptures(uint64_t t_target, int t_piece, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generatePawnsCaptures(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generatePromoCaptures(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const;
    void generateEnPassants(uint64_t t_target, std::vector<Move> &t_moveList, const Board &t_board) const;
private:
    const MagicBitboards& m_lookup;
};