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
public:
    Engine(): mTT{1}, mBoard{Board(STARTPOS)} {}
    ~Engine() {stopSearch();}

    /**
     * @brief Changes the transposition table size to the given dimension 
     * 
     * @param sizeMB The new size in MB
     */
    void resizeTT(int sizeMB);

    /**
     * @brief Sets the starting position to the given one
     * 
     * @param tPosition FEN for the desired starting position
     */
    void setPos(std::string tPosition);

    /**
     * @brief Updates the starting position
     * 
     * @param tMove Legal move to make from the starting position
     */
    void makeMove(std::string tMove);

    /**
     * @brief Starts the search
     * 
     * @param tLimits Search parameters as per SearchLimits specification
     */
    void goSearch(SearchLimits tLimits);
    
    /**
     * @brief Interrupts the search ASAP
     */
    void stopSearch();    

private:
    void mainSearch(int tDepht);
    bool exitSearch();
    
    void printSearchInfo(int tMaxDepth, int64_t tElapsed, int16_t tEval, std::vector<Move> &tPV);
    int16_t alphaBeta(int tDepht, int16_t tAlpha, int16_t tBeta, std::vector<Move> &tPV);
    int16_t quiescence(int16_t tAlpha, int16_t tBeta);

    bool isIllegal(); // opponent side is in check but its not his turn
    bool isCheck();   // opponent side gives check and its your turn
    bool promoThreat();

    bool hashUsageCondition(TTEntry tTTVal, int tDepht, int tAlpha, int tBeta);
    bool threefoldRepetition();
    bool fiftyMove();

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
};

