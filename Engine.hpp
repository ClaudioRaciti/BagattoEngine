#pragma once

#include <vector>
#include <array>
#include <atomic>
#include <thread>
#include <mutex>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "TT.hpp"
#include "LookupTables.hpp"

class Engine
{
private:
    TT m_TT;
    std::vector<std::array<Move, 2>> m_killers;
    Board m_board;
    int m_materialCount;
    uint64_t m_searchedNodes;
    const MoveGenerator m_generator;


    std::atomic<bool> m_gosearch = false;
    std::thread m_thread;
    std::mutex m_engine_mutex;
public:
    Engine(): m_TT{128}, m_board{Board(STARTPOS)} {}
    ~Engine() {stopSearch();}
    void resizeTT(int sizeMB);
    void setPos(std::string t_position);
    void makeMove(std::string t_move);
    void goSearch(int t_depth);
    void stopSearch();

    void mainSearch(int t_depth);
    
private:
    void printSearchInfo(int t_maxDepth, int64_t t_elapsedTime, int16_t eval, std::vector<Move> &t_PV);
    int16_t alphaBeta(int t_depth, int16_t alpha, int16_t beta, std::vector<Move> &t_PV);
    int16_t quiescence(int16_t t_alpha, int16_t t_beta);
    int gamePhase();
    bool isIllegal(); // opponent side is in check but its not his turn
    bool isCheck();   // opponent side gives check and its your turn
    bool promoThreat();
    bool hashUsageCondition(int hashNodeType, int hashScore, int t_alpha, int t_beta);
    std::vector<Move> orderMoves(int t_depth);
};
