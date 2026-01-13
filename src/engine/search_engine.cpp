#include "engine/search_engine.h"
#include <algorithm>
#include <limits>
#include <cmath>

namespace tictactoe {

SearchEngine::SearchEngine(int win_length)
    : moveGen_(win_length), evaluator_(win_length), 
      threatSolver_(win_length), tt_(Config::TT_SIZE_MB),
      win_length_(win_length), timeout_(false) {
}

std::optional<Move> SearchEngine::checkImmediateWin(
    SparseBoard& board, Player player) {
    return moveGen_.checkImmediateWin(board, player);
}

std::optional<Move> SearchEngine::checkImmediateBlock(
    SparseBoard& board, Player player) {
    return moveGen_.checkImmediateBlock(board, player);
}

std::optional<Move> SearchEngine::checkDangerousThreat(
    SparseBoard& board, Player player) {
    return moveGen_.checkDangerousThreat(board, player);
}

int SearchEngine::evaluateTerminal(const SparseBoard& board, Player player) {
    auto history = board.getMoveHistory();
    if (!history.Empty()) {
        const auto& lastMove = history.Back();
        if (board.isWin(lastMove.x, lastMove.y, lastMove.player)) {
            if (lastMove.player == player) {
                return std::numeric_limits<int>::max() / 2;
            } else {
                return std::numeric_limits<int>::min() / 2;
            }
        }
    }
    return evaluator_.evaluatePosition(board, player);
}

bool SearchEngine::hasThreats(const SparseBoard& board, Player player) {
    int minThreatLength = win_length_ - 2;
    if (minThreatLength < 1) {
        minThreatLength = 1;
    }
    
    auto occupied = board.getOccupiedPositions();
    Player opponent = (player == Player::X) ? Player::O : Player::X;
    
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        Player cellPlayer = board.at(pos.x, pos.y);
        
        Player playersToCheck[] = {player, opponent};
        for (int p = 0; p < 2; ++p) {
            if (cellPlayer == playersToCheck[p]) {
                auto patterns = evaluator_.detectPatterns(board, pos.x, pos.y, playersToCheck[p]);
                
                for (int j = 0; j < patterns.GetLength(); ++j) {
                    const auto& pattern = patterns.Get(j);
                    if (pattern.length >= minThreatLength) {
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

void SearchEngine::orderMoves(adt::ArraySequence<Move>& moves, 
                               const std::optional<Move>& pvMove) {
    if (pvMove.has_value()) {
        int pvIndex = -1;
        for (int i = 0; i < moves.GetLength(); ++i) {
            if (moves.Get(i).x == pvMove->x && moves.Get(i).y == pvMove->y) {
                pvIndex = i;
                break;
            }
        }
        if (pvIndex >= 0 && pvIndex > 0) {
            Move temp = moves.Get(0);
            moves.Set(0, moves.Get(pvIndex));
            moves.Set(pvIndex, temp);
        }
    }
    
    if (pvMove.has_value() && moves.GetLength() > 1) {
        adt::ArraySequence<Move> rest;
        for (int i = 1; i < moves.GetLength(); ++i) {
            rest.AppendInPlace(moves.Get(i));
        }
        rest.SortInPlace();
        for (int i = 1; i < moves.GetLength(); ++i) {
            moves.Set(i, rest.Get(i - 1));
        }
    } else {
        moves.SortInPlace();
    }
}

int SearchEngine::quiescence(SparseBoard& board, int alpha, int beta, 
                             Player player, int depth) {
    stats_.nodes_searched++;
    
    if (timeout_ || depth > 4) {
        return evaluator_.evaluatePosition(board, player);
    }
    
    if (board.isTerminal()) {
        return evaluateTerminal(board, player);
    }
    
    int standPat = evaluator_.evaluatePosition(board, player);
    
    if (standPat >= beta) {
        return beta;
    }
    
    if (standPat > alpha) {
        alpha = standPat;
    }
    
    auto candidates = moveGen_.generateCandidates(board, player);
    
    adt::ArraySequence<Move> tacticalMoves;
    for (int i = 0; i < candidates.GetLength(); ++i) {
        const auto& move = candidates.Get(i);
        if (std::abs(move.score) > 1000) {
            tacticalMoves.AppendInPlace(move);
        }
    }
    
    for (int i = 0; i < tacticalMoves.GetLength(); ++i) {
        const auto& move = tacticalMoves.Get(i);
        board.makeMove(move.x, move.y, player);
        Player opponent = (player == Player::X) ? Player::O : Player::X;
        int score = -quiescence(board, -beta, -alpha, opponent, depth + 1);
        board.undoMove(move.x, move.y);
        
        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

int SearchEngine::negamax(SparseBoard& board, int depth, int alpha, int beta,
                         Player player, Move* pv, int pvIndex) {
    stats_.nodes_searched++;
    
    if (timeout_) {
        return 0;
    }
    
    uint64_t hash = board.getZobristHash();
    
    auto ttResult = tt_.probe(hash, depth, alpha, beta);
    if (ttResult.found) {
        if (pv && pvIndex < 20) {
            pv[pvIndex] = ttResult.bestMove;
        }
        return ttResult.score;
    }
    
    if (board.isTerminal() || depth == 0) {
        int score = quiescence(board, alpha, beta, player);
        return score;
    }
    
    auto moves = moveGen_.generateCandidates(board, player);
    if (moves.Empty()) {
        return evaluator_.evaluatePosition(board, player);
    }
    
    auto pvMove = tt_.getPVMove(hash);
    orderMoves(moves, pvMove);
    
    Move bestMove(0, 0);
    int bestScore = std::numeric_limits<int>::min();
    TTFlag flag = TTFlag::UPPER_BOUND;
    bool moveFound = false;
    
    for (int i = 0; i < moves.GetLength(); ++i) {
        const auto& move = moves.Get(i);
        
        if (!board.isEmpty(move.x, move.y)) {
            continue;
        }
        
        moveFound = true;
        board.makeMove(move.x, move.y, player);
        Player opponent = (player == Player::X) ? Player::O : Player::X;
        
        int reduction = 0;
        if (depth > 2) {
            if (i > 3) reduction = 1;
            if (i > 6 && depth > 4) reduction = 2;
            if (i > 10 && depth > 6) reduction = 3;
            
            if (move.score < -1000) {
                reduction += 1;
            }
            
            reduction = std::min(reduction, depth - 1);
        }
        
        int score = -negamax(board, depth - 1 - reduction, -beta, -alpha, 
                            opponent, pv, pvIndex + 1);
        
        if (reduction > 0 && score > alpha) {
            score = -negamax(board, depth - 1, -beta, -alpha, 
                            opponent, pv, pvIndex + 1);
        }
        
        board.undoMove(move.x, move.y);
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            
            if (pv && pvIndex < 20) {
                pv[pvIndex] = move;
            }
        }
        
        alpha = std::max(alpha, score);
        if (alpha >= beta) {
            flag = TTFlag::LOWER_BOUND;
            break;
        }
    }
    
    if (!moveFound) {
        return evaluator_.evaluatePosition(board, player);
    }
    
    if (bestScore <= alpha) {
        flag = TTFlag::UPPER_BOUND;
    } else if (bestScore >= beta) {
        flag = TTFlag::LOWER_BOUND;
    } else {
        flag = TTFlag::EXACT;
    }
    
    tt_.store(hash, bestScore, depth, flag, bestMove);
    
    return bestScore;
}

Move SearchEngine::findBestMove(SparseBoard& board, Player player, int timeMs) {
    stats_ = SearchStats();
    timeout_ = false;
    timer_.reset();
    
    auto history = board.getMoveHistory();
    int movesMade = history.GetLength();
    
    auto winMove = checkImmediateWin(board, player);
    if (winMove.has_value()) {
        stats_.time_ms = timer_.elapsedMs();
        stats_.decision_type = DecisionType::IMMEDIATE_WIN;
        stats_.final_score = std::numeric_limits<int>::max() / 2;
        return *winMove;
    }
    
    auto blockMove = checkImmediateBlock(board, player);
    if (blockMove.has_value()) {
        stats_.time_ms = timer_.elapsedMs();
        stats_.decision_type = DecisionType::IMMEDIATE_BLOCK;
        stats_.final_score = std::numeric_limits<int>::max() / 2 - 1;
        return *blockMove;
    }
    
    auto dangerousThreat = checkDangerousThreat(board, player);
    if (dangerousThreat.has_value()) {
        stats_.time_ms = timer_.elapsedMs();
        stats_.decision_type = DecisionType::DANGEROUS_THREAT;
        stats_.final_score = std::numeric_limits<int>::max() / 2 - 2;
        return *dangerousThreat;
    }
    
    if (movesMade >= 4 && hasThreats(board, player)) {
        auto forcedMove = threatSolver_.findForcedWin(board, player, Config::THREAT_SOLVER_MAX_DEPTH);
        if (forcedMove.has_value()) {
            stats_.time_ms = timer_.elapsedMs();
            stats_.decision_type = DecisionType::THREAT_SOLVER;
            stats_.final_score = std::numeric_limits<int>::max() / 2;
            return *forcedMove;
        }
    }
    
    stats_.decision_type = DecisionType::NEGAMAX_SEARCH;
    
    Move bestMove(0, 0);
    Move previousBestMove(0, 0);
    int previousBestScore = 0;
    int stableIterations = 0;
    bool bestMoveSet = false;
    
    int maxDepth = Config::MAX_DEPTH;
    if (movesMade < 6) {
        maxDepth = std::min(maxDepth, 6);
    } else if (movesMade < 12) {
        maxDepth = std::min(maxDepth, 8);
    }
    
    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (timeout_ || timer_.isTimeout(timeMs)) {
            timeout_ = true;
            break;
        }
        
        Move pv[20];
        for (int i = 0; i < 20; ++i) {
            pv[i] = Move(0, 0);
        }
        
        int bestScore = negamax(board, depth, 
                std::numeric_limits<int>::min(),
                std::numeric_limits<int>::max(),
                player, pv, 0);
        
        if (!timeout_ && board.isEmpty(pv[0].x, pv[0].y)) {
            bestMove = pv[0];
            bestMoveSet = true;
            stats_.depth_reached = depth;
            
            stats_.pv_length = 0;
            for (int i = 0; i < depth && i < 20; ++i) {
                if (pv[i].x == 0 && pv[i].y == 0) break;
                stats_.principal_variation[i] = pv[i];
                stats_.pv_length++;
            }
            
            if (depth >= 3) {
                if (bestMove.x == previousBestMove.x && 
                    bestMove.y == previousBestMove.y &&
                    std::abs(bestScore - previousBestScore) < Config::STABLE_SCORE_THRESHOLD) {
                    stableIterations++;
                    if (stableIterations >= Config::STABLE_ITERATIONS_THRESHOLD) {
                        break;
                    }
                } else {
                    stableIterations = 0;
                }
            }
            
            previousBestMove = bestMove;
            previousBestScore = bestScore;
        }
        
        tt_.incrementAge();
    }
    
    stats_.time_ms = timer_.elapsedMs();
    stats_.final_score = previousBestScore;
    
    if (bestMoveSet && board.isEmpty(bestMove.x, bestMove.y)) {
        return bestMove;
    }
    
    auto candidates = moveGen_.generateCandidates(board, player);
    if (!candidates.Empty()) {
        for (int i = 0; i < candidates.GetLength(); ++i) {
            const auto& move = candidates.Get(i);
            if (board.isEmpty(move.x, move.y)) {
                return move;
            }
        }
    }
    
    auto occupied = board.getOccupiedPositions();
    if (!occupied.Empty()) {
        for (int i = 0; i < occupied.GetLength(); ++i) {
            const auto& pos = occupied.Get(i);
            for (int dx = -2; dx <= 2; ++dx) {
                for (int dy = -2; dy <= 2; ++dy) {
                    if (dx == 0 && dy == 0) continue;
                    int nx = pos.x + dx;
                    int ny = pos.y + dy;
                    if (board.isEmpty(nx, ny)) {
                        return Move(nx, ny, 0);
                    }
                }
            }
        }
    }
    
    return Move(0, 0, 0);
}

} // namespace tictactoe

