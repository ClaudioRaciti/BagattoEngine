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
    void setPos(std::string tPosition);
    void makeMove(std::string tMove);
    void goSearch(SearchLimits tLimits);
    void stopSearch();    
private:
    void mainSearch(int tDepht);
    void printSearchInfo(int tMaxDepth, int64_t tElapsed, int16_t tEval, std::vector<Move> &tPV);
    int16_t alphaBeta(int tDepht, int16_t tAlpha, int16_t tBeta, std::vector<Move> &tPV);
    int16_t quiescence(int16_t tAlpha, int16_t tBeta);
    int gamePhase();
    bool isIllegal(); // opponent side is in check but its not his turn
    bool isCheck();   // opponent side gives check and its your turn
    bool promoThreat();
    bool hashUsageCondition(TTEntry tTTVal, int tDepht, int tAlpha, int tBeta);
    bool threefoldRepetition();
    bool fiftyMove();
    bool exitSearch();
};

