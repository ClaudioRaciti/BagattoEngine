#include "Engine.hpp"
#include "Move.hpp"
#include "TT.hpp"
#include "evaluation.hpp" 
#include "notation.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <stdexcept>

void Engine::resizeTT(int tMBSize)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    mTT.resize(tMBSize);
}

void Engine::setPos(std::string tPosition)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    mBoard = Board(tPosition);
    mGameHist.emplace_back(mBoard.getHash());
}

void Engine::makeMove(std::string tMove)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    std::vector<Move> moveList;
    moveList.reserve(256);
    mGenerator.generate(mBoard, moveList);
    for (auto move : moveList) if (tMove == move.asString()){
        mBoard.makeMove(move);
        mGameHist.emplace_back(mBoard.getHash());
        return;
    }
    throw std::invalid_argument("invalid move");
}

void Engine::goSearch(SearchLimits tLimits)
{
    stopSearch();
    mGoSearch = true;
    mLimits = tLimits;
    int depth = tLimits.infinite ? 99 : tLimits.depth;
    mThread = std::thread(&Engine::mainSearch, this, depth);
}

void Engine::stopSearch()
{
    mGoSearch = false;
    if (mThread.joinable()) mThread.join();
}

void Engine::mainSearch(int tMaxDepth)
{
    const std::lock_guard guard(mEngineMutex);

    static constexpr int16_t windowSize = 50;
    int16_t eval = 0, alpha = CHECKMATE, beta = -CHECKMATE;
    std::vector<Move> PV;
    Move bestmove;

    mKillers.resize(tMaxDepth);
    mGameHist.resize(tMaxDepth);

    for(int depth = 0; depth <= tMaxDepth && !exitSearch(); depth ++){
        int lowFails = 0, highFails = 0;

        do {
            // exponentially widening the aspiration window for each failed search
            if (eval <= alpha) alpha -= windowSize * (1 << ++lowFails);
            if (eval >= beta ) beta  += windowSize * (1 << ++highFails);

            mSearchedNodes = 0;
            auto start = std::chrono::high_resolution_clock::now();
            eval = alphaBeta(depth, alpha, beta, PV);
            auto stop  = std::chrono::high_resolution_clock::now(); 
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

            if (!exitSearch()) printSearchInfo(depth, elapsed, eval, PV);
        } while ((eval <= alpha || eval >= beta) && !exitSearch()); 


        alpha = eval - windowSize;
        beta  = eval + windowSize;

        if(PV.size() && !exitSearch()) bestmove = PV.back();
    }
    
    mGoSearch = false;
    std::cout << "bestmove " << bestmove << std::endl;
}


void Engine::printSearchInfo(int tDepth, int64_t tElapsed, int16_t tEval, std::vector<Move> &tPV)
{
    double elapsedSec = tElapsed / 1000.0;
    uint64_t nps = (elapsedSec > 0) ? static_cast<uint64_t>(mSearchedNodes / elapsedSec) : 0;
    
    std::cout << "info depth " << tDepth << " nodes " << mSearchedNodes << " time " << tElapsed << " nps " << nps;

    // if(eval <= CHECKMATE) std::cout << " mate " << (t_maxDepth - (CHECKMATE - eval)) / 2 + 1 << " ";
    // else if( eval  >= -CHECKMATE) std::cout << " mate -" << (t_maxDepth - (CHECKMATE + eval)) / 2 + 1 << " ";
    // else
    std::cout << " score cp " << tEval;
    
    if (tPV.size()){
        std::cout << " pv ";
        for (auto move = tPV.crbegin(); move != tPV.crend(); move ++) if(move->asString() != "a1a1") std::cout << *move << " ";
    }

    std::cout << std::endl;
}


int16_t Engine::alphaBeta(int tDepth, int16_t tAlpha, int16_t tBeta, std::vector<Move> &tPV){ 
    if (exitSearch() || threefoldRepetition() || fiftyMove()) return DRAW;
    if (tDepth == 0) return quiescence(tAlpha, tBeta);  

    mSearchedNodes +=1;

    // Hash move search    
    uint64_t hashKey = mBoard.getHash();
    auto [ttHit, ttEntry] = mTT.probe(hashKey);
    if (ttHit && hashUsageCondition(ttEntry, tDepth, tAlpha, tBeta)){
            tPV.emplace_back(ttEntry.hashMove); 
            return ttEntry.score;
    }

    Move bestMove;
    int16_t bestScore = CHECKMATE - tDepth; 
    uint8_t bestNodeType = allNode;
    std::vector<Move> line(tDepth);

    // Generating and sorting all the moves
    std::vector<Move> moveList;
    moveList.reserve(256);
    mGenerator.generate(mBoard, moveList);
    auto it = std::partition(moveList.begin(), moveList.end(), [](const Move m){return m.isCapture();});
    std::sort(moveList.begin(), it, [&](const Move m1, const Move m2){
        return mBoard.searchCaptured(m1.to())>mBoard.searchCaptured(m2.to());
    });
    std::partition(it, moveList.end(), [&](const Move m){
        return m == mKillers[tDepth-1][0] || m == mKillers[tDepth-1][1];
    });
    if(ttHit) {
        it = std::find(moveList.begin(), moveList.end(), ttEntry.hashMove);
        if (it != moveList.end()) std::rotate(moveList.begin(), it, it + 1);
    }

    // Searching the moves
    for (Move move : moveList){
        mBoard.makeMove(move);
        mGameHist.emplace_back(mBoard.getHash());
        if(!isIllegal()){
            int16_t score;
            // zero-window search if alpha has already been raised
            if (bestNodeType == pvNode)
                score = -alphaBeta(tDepth - 1, -tAlpha - 1, -tAlpha, line);
            // full window search if alpha hasn't been searched or move could raise alpha
            if (bestNodeType != pvNode || (score > tAlpha && score < tBeta))
                score = -alphaBeta(tDepth - 1, -tBeta, -tAlpha, line);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                if (bestScore > tAlpha) {
                    bestNodeType = pvNode;
                    tAlpha = bestScore;
                    tPV = line;
                    tPV.emplace_back(move);
                }
            }
        }
        mBoard.undoMove(move);
        mGameHist.pop_back();

        if (tAlpha >= tBeta){
            if (!exitSearch() && tDepth >= ttEntry.depht) 
                mTT.insert({hashKey, bestScore, uint8_t(tDepth), cutNode, bestMove});
            if (!move.isCapture() && mKillers[tDepth-1][0] != move){
                mKillers[tDepth-1][1] = mKillers[tDepth-1][0];
                mKillers[tDepth-1][0] = move;
            }
            return bestScore;
        }
    }

    if(!exitSearch() && tDepth >= ttEntry.depht){
        if (bestScore == CHECKMATE - tDepth && !isCheck()) bestScore = DRAW;
        mTT.insert({hashKey, bestScore, uint8_t(tDepth), bestNodeType, bestMove});
    }
    
    return bestScore;
}

int16_t Engine::quiescence(int16_t tAlpha, int16_t tBeta)
{    
    mSearchedNodes += 1;

    static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000}; 
    int16_t standPat = (mBoard.getSideToMove() == white) ? evaluate(mBoard, gamePhase()) : -evaluate(mBoard, gamePhase());
    int16_t bestScore;
    std::vector<Move> moveList;
    moveList.reserve(256);

    if (isCheck()){
        bestScore = CHECKMATE;
        mGenerator.evadeChecks(mBoard, moveList);
    }
    else{
        bestScore = standPat;
        if(bestScore > tAlpha) {
            tAlpha = bestScore; 
            if(tAlpha >= tBeta) return bestScore;
        }
        else if(bestScore + (promoThreat() ? 1800 : 1000) < tAlpha) return bestScore;

        uint64_t enemySet = mBoard.getBitboard(1 - mBoard.getSideToMove());
        mGenerator.generate(enemySet, mBoard, moveList);
        std::sort(moveList.begin(), moveList.end(), [this](const Move& m1, const Move& m2){
            if (m1.isEnPassant()) return false;
            else if (m2.isEnPassant()) return true;
            else return this->mBoard.searchCaptured(m1.to()) > this->mBoard.searchCaptured(m2.to());
        });
    }

    for (const auto& move : moveList){
        mBoard.makeMove(move);
            if(!isIllegal() && standPat + pieceVal[mBoard.getCaptured()] + 200 > tAlpha){ 
            int16_t score = -quiescence(-tBeta, -tAlpha);
            
            if (score > bestScore) {
                bestScore = score; 
                if (bestScore > tAlpha) tAlpha = bestScore;
            }
        }
        mBoard.undoMove(move);

        if(tAlpha >= tBeta) return bestScore;
    }

    return bestScore;
}

int Engine::gamePhase()
{
    constexpr int mgMax = 100 * 16 + 300 * 8 + 500 * 4 + 1000 * 2;

    const int material = (mgMax < mBoard.getMaterialCount()) ? mgMax : mBoard.getMaterialCount();

    return (100 * material) / mgMax;
}

bool Engine::isIllegal()
{
    const int stm = mBoard.getSideToMove();
    const int kingSquare = mBoard.getKingSquare(1 - stm);
    return mGenerator.isSquareAttacked(mBoard, kingSquare, stm);
}

bool Engine::isCheck()
{
    const int stm = mBoard.getSideToMove();
    const int kingSquare = mBoard.getKingSquare(stm);
    return mGenerator.isSquareAttacked(mBoard, kingSquare, 1 - stm);
}

bool Engine::promoThreat()
{
    static constexpr uint64_t seventhRank[2] = {uint64_t(0x00ff000000000000), uint64_t(0x000000000000ff00)};
    const int stm = mBoard.getSideToMove();
    return mBoard.getBitboard(pawn) & mBoard.getBitboard(stm) & seventhRank[stm];
}

bool Engine::hashUsageCondition(TTEntry tTTVal, int tDepht, int tAlpha, int tBeta)
{
    return tTTVal.depht >= tDepht && (
            (tTTVal.nodeType == pvNode)
            || (tTTVal.nodeType == allNode && tTTVal.score <= tAlpha)
            || (tTTVal.nodeType == cutNode && tTTVal.score >= tBeta )
        );
}

bool Engine::threefoldRepetition()
{
    int repetition = 1;
    const int revPlies = mBoard.getHMC(); // number of plies with reversible moves
    const int histSize = mGameHist.size(); // lenght of current game
    const int maxPlies = std::min(histSize, revPlies);
    
    if (maxPlies < 8) return false; // not enough reversible moves for threefold rep 
    for (int ply = 2; ply <= maxPlies; ply += 2){ // checks every even spaced key for repetition (excluding the second last)
        if(mGameHist[(histSize - 1) - ply] ==  mGameHist.back()) {
            repetition += 1;
            if(repetition == 3) return true;
        }
    }
    return false;
}

bool Engine::fiftyMove()
{
    const int revPlies = mBoard.getHMC(); // number of plies with reversible moves
    return revPlies >= 100;
}

bool Engine::exitSearch()
{
    if (!mGoSearch.load()) 
        return true;
    else if(mLimits.infinite)
        return false;
    else if(mLimits.movetime)
        return (now() - mLimits.timestart) > mLimits.movetime;
    else if(mLimits.nodes)
        return mSearchedNodes > mLimits.nodes;
    else 
        return false;
}
