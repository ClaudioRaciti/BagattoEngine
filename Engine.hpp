#pragma once

#include <vector>
#include <array>
#include <atomic>
#include <thread>
#include <mutex>

#include "Board.hpp"
#include "Move.hpp"
#include "MoveGenerator.hpp"
#include "utils.hpp"
#include "TT.hpp"

class Engine
{
private:
    const MoveGenerator mGenerator;
    std::vector<std::array<Move, 2>> mKillers;
    std::vector<uint64_t> mGameHist;
    TT mTT;
    Board mBoard;
    uint64_t mSearchedNodes;
    SearchLimits mLimits;

    std::atomic<bool> mGoSearch = false;
    std::thread mThread;
    std::mutex mEngineMutex;
public:
    Engine(): mTT{1}, mBoard{Board(STARTPOS)} {}
    ~Engine() {stopSearch();}
    void resizeTT(int sizeMB);
    void setPos(std::string t_position);
    void makeMove(std::string t_move);
    void goSearch(SearchLimits tLimits);
    void stopSearch();    
private:
    void mainSearch(int t_depth);
    void printSearchInfo(int t_maxDepth, int64_t t_elapsedTime, int16_t eval, std::vector<Move> &t_PV);
    int16_t alphaBeta(int t_depth, int16_t alpha, int16_t beta, std::vector<Move> &t_PV);
    int16_t quiescence(int16_t t_alpha, int16_t t_beta);
    int gamePhase();
    bool isIllegal(); // opponent side is in check but its not his turn
    bool isCheck();   // opponent side gives check and its your turn
    bool promoThreat();
    bool hashUsageCondition(int hashNodeType, int hashScore, int t_alpha, int t_beta);
    bool threefoldRepetition();
    bool fiftyMove();
    bool exitSearch();
};

