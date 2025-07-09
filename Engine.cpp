#include "Engine.hpp"
#include "evaluation.hpp" 
#include "notation.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <stdexcept>

void Engine::resizeTT(int sizeMB)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    mTT.resize(sizeMB);
}

void Engine::setPos(std::string t_position)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    mBoard = Board(t_position);
    mGameHist.emplace_back(mBoard.getHash());
}

void Engine::makeMove(std::string t_move)
{
    stopSearch();
    const std::lock_guard guard(mEngineMutex);
    std::vector<Move> moveList;
    moveList.reserve(256);
    mGenerator.generateMoves(mBoard, moveList);
    for (auto move : moveList) if (t_move == move.asString()){
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
    int depth = tLimits.infinite ? INT32_MAX : tLimits.depth;
    mThread = std::thread(&Engine::mainSearch, this, depth);
}

void Engine::stopSearch()
{
    mGoSearch = false;
    if (mThread.joinable()) mThread.join();
}

void Engine::mainSearch(int t_maxDepth)
{
    const std::lock_guard guard(mEngineMutex);

    static constexpr int16_t windowSize = 50;
    int16_t eval = 0, alpha = CHECKMATE, beta = -CHECKMATE;
    std::vector<Move> PV;
    Move bestmove;

    mKillers.resize(t_maxDepth);
    mGameHist.resize(t_maxDepth);

    for(int depth = 0; depth <= t_maxDepth && !exitSearch(); depth ++){
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


void Engine::printSearchInfo(int t_maxDepth, int64_t t_elapsedTime, int16_t eval, std::vector<Move> &t_PV)
{
    double elapsedSec = t_elapsedTime / 1000.0;
    uint64_t nps = (elapsedSec > 0) ? static_cast<uint64_t>(mSearchedNodes / elapsedSec) : 0;
    
    std::cout << "info depth " << t_maxDepth << " nodes " << mSearchedNodes << " time " << t_elapsedTime << " nps " << nps;

    // if(eval <= CHECKMATE) std::cout << " mate " << (t_maxDepth - (CHECKMATE - eval)) / 2 + 1 << " ";
    // else if( eval  >= -CHECKMATE) std::cout << " mate -" << (t_maxDepth - (CHECKMATE + eval)) / 2 + 1 << " ";
    // else
    std::cout << " score cp " << eval;
    
    if (t_PV.size()){
        std::cout << " pv ";
        for (auto move = t_PV.crbegin(); move != t_PV.crend(); move ++) if(move->asString() != "a1a1") std::cout << *move << " ";
    }

    std::cout << std::endl;
}


int16_t Engine::alphaBeta(int tDepth, int16_t tAlpha, int16_t tBeta, std::vector<Move> &tPV){ 
    if (exitSearch() || threefoldRepetition() || fiftyMove()) return DRAW;
    if (tDepth == 0) return quiescence(tAlpha, tBeta);  

    mSearchedNodes +=1;

    Move bestMove;
    int16_t bestScore = CHECKMATE - tDepth; 
    int bestNodeType = allNode;
    std::vector<Move> line(tDepth);
    
    uint64_t hashKey = mBoard.getHash();
    // Hash move search
    if (mTT.contains(hashKey)){
        TTValue hashValue = mTT.getValue(hashKey);
        Move hashMove  = mTT.getMove(hashKey);
        int16_t hashScore = (hashValue.score() == CHECKMATE) ? CHECKMATE - tDepth : hashValue.score();

        if (hashValue.depth() >= tDepth && hashUsageCondition(hashValue.nodeType(), hashScore, tAlpha, tBeta)){
            tPV = line;
            tPV.emplace_back(hashMove); 
            return hashScore;
        }

        mBoard.makeMove(hashMove);
        mGameHist.emplace_back(mBoard.getHash());
        if(!isIllegal()){
            int16_t score;
            if (hashValue.nodeType() == pvNode){
                score = -alphaBeta(tDepth - 1, -tAlpha - 1, -tAlpha, line);
                if (score > tAlpha && score < tBeta){
                    score = -alphaBeta(tDepth -1, -tBeta, -tAlpha, line);
                }
            }
            else{
                score = -alphaBeta(tDepth - 1, -tBeta, -tAlpha, line);
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = hashMove;
                if (bestScore > tAlpha) {
                    bestNodeType = pvNode;
                    tAlpha = bestScore;
                    tPV = line;
                    tPV.emplace_back(hashMove);
                }
            }
        }
        mBoard.undoMove(hashMove);
        mGameHist.pop_back();

        if (tAlpha >= tBeta){
            if (!exitSearch()) mTT.insert(hashKey, bestScore, tDepth, cutNode, bestMove);
            if (!hashMove.isCapture() && mKillers[tDepth-1][0] != hashMove){
                mKillers[tDepth-1][1] = mKillers[tDepth-1][0];
                mKillers[tDepth-1][0] = hashMove;
            }
            return bestScore;
        }
    }

    std::vector<Move> captureList;
    captureList.reserve(256);
    mGenerator.generateCaptures(mBoard, captureList);
    std::sort(captureList.begin(), captureList.end(), [this](const Move& m1, const Move& m2){
        if (m1.isEnPassant()) return false;
        else if (m2.isEnPassant()) return true;
        else return this->mBoard.searchCaptured(m1.to()) > this->mBoard.searchCaptured(m2.to());
    });

    for(const auto& move : captureList){
        mBoard.makeMove(move);
        mGameHist.emplace_back(mBoard.getHash());
        if (!isIllegal()){
            int16_t score;
            if (bestNodeType == pvNode){
                score = -alphaBeta(tDepth - 1, -tAlpha - 1, -tAlpha, line);
                if (score > tAlpha && score < tBeta){
                    score = -alphaBeta(tDepth -1, -tBeta, -tAlpha, line);
                }
            }
            else{
                score = -alphaBeta(tDepth - 1, -tBeta, -tAlpha, line);
            }

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
            if (!exitSearch()) mTT.insert(hashKey, bestScore, tDepth, cutNode, bestMove);
            return bestScore;
        }
    }

    std::vector<Move> quietList;
    quietList.reserve(256);
    mGenerator.generateQuiets(mBoard, quietList);
    std::partition(quietList.begin(), quietList.end(), [&](const Move& m){
        return m == mKillers[tDepth-1][0] || m == mKillers[tDepth-1][1];
    });

    for(const auto& move : quietList){
        mBoard.makeMove(move);
        mGameHist.emplace_back(mBoard.getHash());
        if (!isIllegal()){
            int16_t score;
            if (bestNodeType == pvNode){
                score = -alphaBeta(tDepth - 1, -tAlpha - 1, -tAlpha, line);
                if (score > tAlpha && score < tBeta){
                    score = -alphaBeta(tDepth -1, -tBeta, -tAlpha, line);
                }
            }
            else{
                score = -alphaBeta(tDepth - 1, -tBeta, -tAlpha, line);
            }

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
            if (!exitSearch()) mTT.insert(hashKey, bestScore, tDepth, cutNode, bestMove);
            if (mKillers[tDepth-1][0] != move){
                mKillers[tDepth-1][1] = mKillers[tDepth-1][0];
                mKillers[tDepth-1][0] = move;
            }
            return bestScore;
        }
    }

    if(!exitSearch()){
        // In
        if (bestScore != CHECKMATE - tDepth)
            mTT.insert(hashKey, bestScore, tDepth, bestNodeType, bestMove); 
        else if (isCheck())
            mTT.insert(hashKey, CHECKMATE, INT16_MAX, pvNode, bestMove); // Rescale checkmate value for tt insertion
        else {
            bestScore = DRAW; 
            mTT.insert(hashKey, DRAW, INT16_MAX, pvNode, bestMove);
        } // Stalemate
    }
    
    return bestScore;
}

int16_t Engine::quiescence(int16_t t_alpha, int16_t t_beta)
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
        if(bestScore > t_alpha) {
            t_alpha = bestScore; 
            if(t_alpha >= t_beta) return bestScore;
        }
        else if(bestScore + (promoThreat() ? 1800 : 1000) < t_alpha) return bestScore;

        mGenerator.generateCaptures(mBoard, moveList);
        std::sort(moveList.begin(), moveList.end(), [this](const Move& m1, const Move& m2){
            if (m1.isEnPassant()) return false;
            else if (m2.isEnPassant()) return true;
            else return this->mBoard.searchCaptured(m1.to()) > this->mBoard.searchCaptured(m2.to());
        });
    }

    for (const auto& move : moveList){
        mBoard.makeMove(move);
            if(!isIllegal() && standPat + pieceVal[mBoard.getCaptured()] + 200 > t_alpha){ 
            int16_t score = -quiescence(-t_beta, -t_alpha);
            
            if (score > bestScore) {
                bestScore = score; 
                if (bestScore > t_alpha) t_alpha = bestScore;
            }
        }
        mBoard.undoMove(move);

        if(t_alpha >= t_beta) return bestScore;
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

bool Engine::hashUsageCondition(int hashNodeType, int hashScore, int t_alpha, int t_beta)
{
    return (hashNodeType == pvNode)
        || (hashNodeType == allNode && hashScore <= t_alpha)
        || (hashNodeType == cutNode && hashScore >= t_beta );
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
