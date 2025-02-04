#include "Engine.hpp"
#include "evaluation.hpp" 
#include "notation.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cassert>
#include <chrono>

int16_t Engine::getEval(std::string t_position, int t_depth)
{   
    static constexpr int pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    m_board = Board(t_position);
    m_materialCount = 0;
    m_killers.reserve(t_depth);
    for (int piece = pawn; piece < king; piece ++) 
        m_materialCount += popCount(m_board.getBitboard(piece)) * pieceVal[piece];
    return iterativeDeepening(0, t_depth, INT16_MIN, INT16_MAX);
}

int16_t Engine::debugQuiescence(std::string t_position)
{
    m_board = Board(t_position);
    return quiescence(INT16_MIN, INT16_MAX);
}

int16_t Engine::iterativeDeepening(int t_depth, int t_maxDepth, int16_t t_alpha, int16_t t_beta)
{
    std::vector<Move> PV;
    constexpr int16_t windowSize = 50;
    std:: cout << "\n\nalpha = " << t_alpha << " beta = " << t_beta << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    int16_t result = alphaBeta(0, t_depth, t_alpha, t_beta, PV);
    std::cout << "At depth " << t_depth << " evaluation is: " << result << "\n";
    
    int lowFails = 0, highFails = 0;
    while (result <= t_alpha || result >= t_beta) {
        std::cout << "\nSearching again..." << std::endl;
        if (result <= t_alpha) t_alpha -= windowSize * (1 << ++lowFails);
        if (result >= t_beta ) t_beta  += windowSize * (1 << ++highFails);
        std:: cout << "\nalpha = " << t_alpha << " beta = " << t_beta << std::endl;
        result = alphaBeta(0, t_depth, t_alpha, t_beta, PV);
        std::cout << "At depth " << t_depth << " evaluation is: " << result << "\n";
    }
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fp_ms = stop - start;
    std::cout << "Principal variation: ";
    std::cout << "1." << (m_board.getSideToMove() == white ? " " : ".. ");
    int nMove = 1;
    for (auto move = PV.crbegin(); move < PV.crend(); move ++){
        nMove += 1;
        if(m_board.getSideToMove() == white && nMove%2 == 0 && nMove > 2) std::cout << nMove/2 << ". ";
        if(m_board.getSideToMove() == black && nMove%2 == 1) std::cout << nMove/2 + 1<< ". ";
        std::cout << *move << " ";
    }
    std::cout << "\n("<< fp_ms.count() <<" secondi)\n";
    if (t_depth == t_maxDepth) return result;

    return iterativeDeepening(t_depth + 1, t_maxDepth, result - windowSize, result + windowSize);
}

int16_t Engine::alphaBeta(int t_depth, int t_maxDepth, int16_t t_alpha, int16_t t_beta, std::vector<Move> &t_PV)
{
    if(t_depth == t_maxDepth) return quiescence(t_alpha, t_beta);

    static constexpr int pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    int16_t  hashScore, bestScore = CHECKMATE;
    Move hashMove, bestMove;
    int  hashNodeType, bestNodeType = allNode;
    int  hashDepth;
    std::vector<Move> line(t_maxDepth - t_depth - 1);
    
    if(m_TTable.contains(m_board, hashScore, hashDepth, hashNodeType, hashMove)) {
        switch (hashNodeType){
            case pvNode:  if(hashDepth >= (t_maxDepth - t_depth)) return hashScore; break;
            case allNode: if(hashDepth >= (t_maxDepth - t_depth) && hashScore <= t_alpha) return hashScore; break;
            case cutNode: if(hashDepth >= (t_maxDepth - t_depth) && hashScore >= t_beta ) return hashScore; break;
        }
        
        m_board.makeMove(hashMove);
        if(!isIllegal()){
            m_materialCount -= pieceVal[hashMove.captured()];
            int16_t score = -alphaBeta(t_depth + 1, t_maxDepth, -t_beta, -t_alpha, line);
            m_materialCount += pieceVal[hashMove.captured()];

            if (score > bestScore) {
                bestScore = score;
                bestMove = hashMove;
                if (bestScore > t_alpha) {
                    bestNodeType = pvNode;
                    t_alpha = bestScore;
                    t_PV = line;
                    t_PV.emplace_back(hashMove);
                }
            }
        }
        m_board.undoMove(hashMove);
        if (t_alpha >= t_beta){
            m_TTable.insert(m_board, bestScore, (t_maxDepth - t_depth), cutNode, bestMove);
            return bestScore;
        }

    }
    
    std::vector<Move> captureList = m_generator.generateCaptures(m_board);
    std::sort(captureList.begin(), captureList.end(), [&](const Move& a, const Move& b){return a.asInt() > b.asInt();});
    for (auto move : captureList){
        m_board.makeMove(move);
        if(!isIllegal()){
            m_materialCount -= pieceVal[move.captured()];
            int16_t score = -alphaBeta(t_depth + 1, t_maxDepth, -t_beta, -t_alpha, line);
            m_materialCount += pieceVal[move.captured()];

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
        if (t_alpha >= t_beta){
            m_TTable.insert(m_board, bestScore, (t_maxDepth - t_depth), cutNode, bestMove);
            return bestScore;
        }
    }

    std::vector<Move> quietList = m_generator.generateQuiets(m_board);
    std::partition(quietList.begin(), quietList.end(), [&](const Move& m){return m == m_killers[t_depth][0] || m == m_killers[t_depth][1];});
    for (auto move : quietList){
        m_board.makeMove(move);
        if(!isIllegal()){
            int16_t score = -alphaBeta(t_depth + 1, t_maxDepth, -t_beta, -t_alpha, line);
            
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
        if (t_alpha >= t_beta){
            m_TTable.insert(m_board, bestScore, (t_maxDepth - t_depth), cutNode, bestMove);
            if(m_killers[t_depth][0] != move){
                m_killers[t_depth][1] = m_killers[t_depth][0];
                m_killers[t_depth][0] = move;
            }
            return bestScore;
        }
    }


    if (bestScore == CHECKMATE && !isCheck()) bestScore = 0;

    m_TTable.insert(m_board, bestScore, (t_maxDepth - t_depth), bestNodeType, bestMove);

    if (bestScore == CHECKMATE) bestScore += t_depth;
    return bestScore;
}

int16_t Engine::quiescence(int16_t t_alpha, int16_t t_beta)
{
    if (isCheck()) return evadeChecks(t_alpha, t_beta);

    static constexpr int pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    int16_t standPat = evaluate(m_board, gamePhase()) * (m_board.getSideToMove() == white ? 1 : -1);
    int16_t bestScore = standPat;

    
    if(bestScore > t_alpha) {t_alpha = bestScore; if(t_alpha >= t_beta) return bestScore;}
    else if(bestScore + pieceVal[queen] + 200 < t_alpha) return bestScore;

    std::vector<Move> moveList = m_generator.generateCaptures(m_board);
    std::sort(moveList.begin(), moveList.end(), [](Move a, Move b){return a.asInt() > b.asInt();});

    for (auto movePtr = moveList.begin(); t_alpha < t_beta && movePtr != moveList.end(); movePtr ++){
        m_board.makeMove(*movePtr);
        if(!isIllegal()){
            int16_t score = CHECKMATE;         
            if (standPat + pieceVal[movePtr->captured()] + 200 > t_alpha ){ 
                m_materialCount -= pieceVal[movePtr->captured()];
                score = -quiescence(-t_beta, -t_alpha);
                m_materialCount += pieceVal[movePtr->captured()];

                if (score > bestScore) {bestScore = score; if (bestScore > t_alpha) t_alpha = bestScore;}
            }
        }
        m_board.undoMove(*movePtr);
    }

    return bestScore;
}

int16_t Engine::evadeChecks(int16_t t_alpha, int16_t t_beta)
{
    static constexpr int pieceVal[7] = {0, 0, 100, 300, 300, 500, 1000};
    int16_t standPat = evaluate(m_board, gamePhase()) * (m_board.getSideToMove() == white ? 1 : -1);
    int16_t bestScore = CHECKMATE;
    std::vector<Move> moveList = m_generator.evadeCheck(m_board);

    for (auto movePtr = moveList.begin(); t_alpha < t_beta && movePtr != moveList.end(); movePtr ++){
        m_board.makeMove(*movePtr);
        if(!isIllegal()){
            int16_t score = CHECKMATE;
            if(!movePtr->isCapture() && standPat + 200 > t_alpha){
                score = -quiescence(-t_beta, - t_alpha);
                if (score > bestScore) {bestScore = score; if (score > t_alpha) t_alpha = score;}
            }            
            else if (standPat + pieceVal[movePtr->captured()] + 200 > t_alpha ){ 
                m_materialCount -= pieceVal[movePtr->captured()];
                score = -quiescence(-t_beta, -t_alpha);
                m_materialCount += pieceVal[movePtr->captured()];

                if (score > bestScore) {bestScore = score; if (score > t_alpha) t_alpha = score;}
            }
        }
        m_board.undoMove(*movePtr);
    }

    return bestScore;
}

int Engine::gamePhase()
{
    constexpr int mgMax = 100 * 16 + 300 * 8 + 500 * 4 + 1000 * 2;

    int material = (mgMax < m_materialCount) ? mgMax : m_materialCount;

    return (100 * material) / mgMax;
}

bool Engine::isIllegal()
{
    int sideToMove = m_board.getSideToMove();
    int kingSquare = m_board.getKingSquare(1 - sideToMove);
    return isSqAttacked(kingSquare, sideToMove);
}

bool Engine::isCheck()
{
    int sideToMove = m_board.getSideToMove();
    int kingSquare = m_board.getKingSquare(sideToMove);
    return isSqAttacked(kingSquare, 1 - sideToMove);
}

bool Engine::isSqAttacked(int t_square, int t_attackingSide)
{    
    uint64_t occupied = m_board.getBitboard(white) | m_board.getBitboard(black);
    uint64_t pawnsSet = m_board.getBitboard(pawn) & m_board.getBitboard(t_attackingSide);
    if ((m_lookup.pawnAttacks(t_square, 1-t_attackingSide) & pawnsSet) != 0) return true;


    for (int piece = knight; piece <= king; piece ++){
        uint64_t pieceSet = m_board.getBitboard(piece) & m_board.getBitboard(t_attackingSide);
        if((m_lookup.getAttacks(piece, t_square, occupied) & pieceSet) != 0) return true;
    } 

    return false;
}
