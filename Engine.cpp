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
    const std::lock_guard guard(m_engine_mutex);
    m_TT.resize(sizeMB);
}

void Engine::setPos(std::string t_position)
{
    stopSearch();
    const std::lock_guard guard(m_engine_mutex);
    m_board = Board(t_position);
    m_gameHist.emplace_back(m_board.getHash());
}

void Engine::makeMove(std::string t_move)
{
    stopSearch();
    const std::lock_guard guard(m_engine_mutex);
    std::vector<Move> moveList;
    moveList.reserve(256);
    m_generator.generateMoves(m_board, moveList);
    for (auto move : moveList) if (t_move == move.asString()){
        m_board.makeMove(move);
        m_gameHist.emplace_back(m_board.getHash());
        return;
    }
    throw std::invalid_argument("invalid move");
}

void Engine::goSearch(int t_depth)
{
    stopSearch();
    m_gosearch = true;
    m_thread = std::thread(&Engine::mainSearch, this, t_depth);
}

void Engine::stopSearch()
{
    m_gosearch = false;
    if (m_thread.joinable()) m_thread.join();
}

void Engine::mainSearch(int t_maxDepth)
{
    const std::lock_guard guard(m_engine_mutex);

    static constexpr int16_t windowSize = 50;
    int16_t eval = 0, alpha = CHECKMATE, beta = -CHECKMATE;
    std::vector<Move> PV;
    Move bestmove;

    m_killers.resize(t_maxDepth);

    for(int depth = 0; depth <= t_maxDepth && m_gosearch.load(); depth ++){
        int lowFails = 0, highFails = 0;

        do {
            if (eval <= alpha) alpha -= windowSize * (1 << ++lowFails);
            if (eval >= beta ) beta  += windowSize * (1 << ++highFails);

            m_searchedNodes = 0;
            auto start = std::chrono::high_resolution_clock::now();
            eval = alphaBeta(depth, alpha, beta, PV);
            auto stop  = std::chrono::high_resolution_clock::now(); 
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

            if (m_gosearch.load()) printSearchInfo(depth, elapsed, eval, PV);
        } while (eval <= alpha || eval >= beta); 


        alpha = eval - windowSize;
        beta  = eval + windowSize;

        if(PV.size() && m_gosearch.load()) bestmove = PV.back();
    }
    
    m_gosearch = false;
    std::cout << "bestmove " << bestmove << std::endl;
}


void Engine::printSearchInfo(int t_maxDepth, int64_t t_elapsedTime, int16_t eval, std::vector<Move> &t_PV)
{
    double elapsedSec = t_elapsedTime / 1000.0;
    uint64_t nps = (elapsedSec > 0) ? static_cast<uint64_t>(m_searchedNodes / elapsedSec) : 0;
    
    std::cout << "info depth " << t_maxDepth << " nodes " << m_searchedNodes << " time " << t_elapsedTime << " nps " << nps;

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


int16_t Engine::alphaBeta(int t_depth, int16_t t_alpha, int16_t t_beta, std::vector<Move> &t_PV){
    if (m_gosearch.load() == false) return 0;
    if (t_depth == 0) return quiescence(t_alpha, t_beta);   
    if (drawCondition()) return 0;

    m_searchedNodes +=1;

    Move hashMove, bestMove;
    int16_t  hashScore, bestScore; 
    int bestNodeType = allNode;
    std::vector<Move> line(t_depth);

    bestScore = CHECKMATE - t_depth;

    // Hash move search
    if (m_TT.contains(m_board)){
        hashMove = m_TT.getMove(m_board);
        hashScore = m_TT.getScore(m_board);
        
        if (hashScore == CHECKMATE) hashScore -= t_depth;

        if (m_TT.getDepth(m_board) >= t_depth && hashUsageCondition(m_TT.getNodeType(m_board), hashScore, t_alpha, t_beta)){
            t_PV = line;
            t_PV.emplace_back(hashMove); 
            return hashScore;
        }
    }

    std::vector<Move> captureList;
    captureList.reserve(256);
    m_generator.generateCaptures(m_board, captureList);
    std::sort(captureList.begin(), captureList.end(), [this](const Move& m1, const Move& m2){
        if (m1.isEnPassant()) return false;
        else if (m2.isEnPassant()) return true;
        else return this->m_board.searchCaptured(m1.to()) > this->m_board.searchCaptured(m2.to());
    });
    if (hashMove.isInit()) {
        if (hashMove.isCapture()) std::partition(captureList.begin(), captureList.end(), [hashMove](const Move& m){return m == hashMove;});
        else captureList.insert(captureList.begin(), hashMove);
    }

    for(const auto& move : captureList){
        m_board.makeMove(move);
        m_gameHist.emplace_back(m_board.getHash());
        if (!isIllegal()){
            int16_t score;
            if (bestNodeType == pvNode){
                score = -alphaBeta(t_depth - 1, -t_alpha - 1, -t_alpha, line);
                if (score > t_alpha && score < t_beta){
                    score = -alphaBeta(t_depth -1, -t_beta, -t_alpha, line);
                }
            }
            else{
                score = -alphaBeta(t_depth - 1, -t_beta, -t_alpha, line);
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                if (bestScore > t_alpha) {
                    bestNodeType = pvNode;
                    t_alpha = bestScore;
                    t_PV = line;
                    t_PV.emplace_back(move);
                }
            }
        }
        m_board.undoMove(move);
        m_gameHist.pop_back();

        if (t_alpha >= t_beta){
            if (m_gosearch.load()) m_TT.insert(m_board, bestScore, t_depth, cutNode, bestMove);
            return bestScore;
        }
    }

    std::vector<Move> quietList;
    quietList.reserve(256);
    m_generator.generateQuiets(m_board, quietList);
    std::partition(quietList.begin(), quietList.end(), [&](const Move& m){return m == m_killers[t_depth-1][0] || m == m_killers[t_depth-1][1];});

    for(const auto& move : quietList){
        m_board.makeMove(move);
        m_gameHist.emplace_back(m_board.getHash());
        if (!isIllegal()){
            int16_t score;
            if (bestNodeType == pvNode){
                score = -alphaBeta(t_depth - 1, -t_alpha - 1, -t_alpha, line);
                if (score > t_alpha && score < t_beta){
                    score = -alphaBeta(t_depth -1, -t_beta, -t_alpha, line);
                }
            }
            else{
                score = -alphaBeta(t_depth - 1, -t_beta, -t_alpha, line);
            }

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
                if (bestScore > t_alpha) {
                    bestNodeType = pvNode;
                    t_alpha = bestScore;
                    t_PV = line;
                    t_PV.emplace_back(move);
                }
            }
        }
        m_board.undoMove(move);
        m_gameHist.pop_back();

        if (t_alpha >= t_beta){
            if (m_gosearch.load()) m_TT.insert(m_board, bestScore, t_depth, cutNode, bestMove);
            if (m_killers[t_depth-1][0] != move){
                m_killers[t_depth-1][1] = m_killers[t_depth-1][0];
                m_killers[t_depth-1][0] = move;
            }
            return bestScore;
        }
    }

    if(m_gosearch.load()){
        if (bestScore != CHECKMATE - t_depth) m_TT.insert(m_board, bestScore, t_depth, bestNodeType, bestMove); // Default
        else if (isCheck()) m_TT.insert(m_board, CHECKMATE, INT16_MAX, pvNode, bestMove); // Rescale checkmate value for tt insertion
        else {bestScore = 0; m_TT.insert(m_board, 0, INT16_MAX, pvNode, bestMove);} // Stalemate
    }
    
    return bestScore;
}

int16_t Engine::quiescence(int16_t t_alpha, int16_t t_beta)
{    
    m_searchedNodes += 1;

    static constexpr int16_t pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000}; 
    int16_t standPat = evaluate(m_board, gamePhase()) * (m_board.getSideToMove() == white ? 1 : -1);
    int16_t bestScore;
    std::vector<Move> moveList;
    moveList.reserve(256);

    if (isCheck()){
        bestScore = CHECKMATE;
        m_generator.evadeChecks(m_board, moveList);
    }
    else{
        bestScore = standPat;
        if(bestScore > t_alpha) {
            t_alpha = bestScore; 
            if(t_alpha >= t_beta) return bestScore;
        }
        else if(bestScore + (promoThreat() ? 1800 : 1000) < t_alpha) return bestScore;
        m_generator.generateCaptures(m_board, moveList);
    

        std::sort(moveList.begin(), moveList.end(), [this](const Move& m1, const Move& m2){
            if (m1.isEnPassant()) return false;
            else if (m2.isEnPassant()) return true;
            else return this->m_board.searchCaptured(m1.to()) > this->m_board.searchCaptured(m2.to());
        });
    }

    for (const auto& move : moveList){
        m_board.makeMove(move);
            if(!isIllegal() && standPat + pieceVal[m_board.getCaptured()] + 200 > t_alpha){ 
            int16_t score = -quiescence(-t_beta, -t_alpha);
            
            if (score > bestScore) {
                bestScore = score; 
                if (bestScore > t_alpha) t_alpha = bestScore;
            }
        }
        m_board.undoMove(move);

        if(t_alpha >= t_beta) return bestScore;
    }

    return bestScore;
}

int Engine::gamePhase()
{
    constexpr int mgMax = 100 * 16 + 300 * 8 + 500 * 4 + 1000 * 2;

    const int material = (mgMax < m_board.getMaterialCount()) ? mgMax : m_board.getMaterialCount();

    return (100 * material) / mgMax;
}

bool Engine::isIllegal()
{
    const int stm = m_board.getSideToMove();
    const int kingSquare = m_board.getKingSquare(1 - stm);
    return m_generator.isSquareAttacked(m_board, kingSquare, stm);
}

bool Engine::isCheck()
{
    const int stm = m_board.getSideToMove();
    const int kingSquare = m_board.getKingSquare(stm);
    return m_generator.isSquareAttacked(m_board, kingSquare, 1 - stm);
}

bool Engine::promoThreat()
{
    static constexpr uint64_t seventhRank[2] = {uint64_t(0x00ff000000000000), uint64_t(0x000000000000ff00)};
    const int stm = m_board.getSideToMove();
    return m_board.getBitboard(pawn) & m_board.getBitboard(stm) & seventhRank[stm];
}

bool Engine::hashUsageCondition(int hashNodeType, int hashScore, int t_alpha, int t_beta)
{
    return (hashNodeType == pvNode)
        || (hashNodeType == allNode && hashScore <= t_alpha)
        || (hashNodeType == cutNode && hashScore >= t_beta );
}

bool Engine::drawCondition()
{
    int repetition = 1;
    const int revPlies = m_board.getHMC(); // number of plies with reversible moves
    const int histSize = m_gameHist.size(); // lenght of current game
    const int maxPlies = std::min(histSize, revPlies);

    if (revPlies >= 100) return true; // 50 move rule
    if (maxPlies < 8) return false; // not enough reversible moves for threefold rep 
    for (int ply = 2; ply <= maxPlies; ply += 2){ // checks every even spaced key for repetition (excluding the second last)
        if(m_gameHist[(histSize - 1) - ply] ==  m_gameHist.back()) {
            repetition += 1;
            if(repetition == 3) return true;
        }
    }
    return false;
}
