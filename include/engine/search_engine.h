#pragma once

#include "board/sparse_board.h"
#include "engine/move_generator.h"
#include "engine/evaluator.h"
#include "engine/threat_solver.h"
#include "engine/transposition_table.h"
#include "utils/timer.h"
#include "engine/config.h"
#include "adt/sequence.h"
#include <optional>
#include <cstdint>
#include <array>

namespace tictactoe {

enum class DecisionType {
    IMMEDIATE_WIN,
    IMMEDIATE_BLOCK,
    DANGEROUS_THREAT,
    THREAT_SOLVER,
    NEGAMAX_SEARCH
};

class SearchStats {
public:
    SearchStats() : nodes_searched_(0), depth_reached_(0), time_ms_(0), pv_length_(0),
                    decision_type_(DecisionType::NEGAMAX_SEARCH), final_score_(0) {
        for (int i = 0; i < 20; ++i) {
            principal_variation_[i] = Move(0, 0);
        }
    }
    
    int getNodesSearched() const { return nodes_searched_; }
    int getDepthReached() const { return depth_reached_; }
    int getTimeMs() const { return time_ms_; }
    DecisionType getDecisionType() const { return decision_type_; }
    int getFinalScore() const { return final_score_; }
    int getPvLength() const { return pv_length_; }
    Move getPrincipalVariation(int index) const {
        if (index >= 0 && index < 20) {
            return principal_variation_[index];
        }
        return Move(0, 0);
    }
    
    friend class SearchEngine;
    
private:
    int nodes_searched_;
    int depth_reached_;
    int time_ms_;
    Move principal_variation_[20];
    int pv_length_;
    DecisionType decision_type_;
    int final_score_;
};

class SearchEngine {
public:
    explicit SearchEngine(int win_length = Config::WIN_LENGTH);
    
    Move findBestMove(SparseBoard& board, Player player, int timeMs = Config::DEFAULT_TIME_MS);
    SearchStats getStats() const { return stats_; }
    void clearTT() { tt_.clear(); }
    
private:
    MoveGenerator moveGen_;
    Evaluator evaluator_;
    ThreatSolver threatSolver_;
    TranspositionTable tt_;
    Timer timer_;
    SearchStats stats_;
    int win_length_;
    bool timeout_;
    
    int negamax(SparseBoard& board, int depth, int alpha, int beta, 
                Player player, Move* pv, int pvIndex);
    int quiescence(SparseBoard& board, int alpha, int beta, Player player, int depth = 0);
    void orderMoves(adt::ArraySequence<Move>& moves, const std::optional<Move>& pvMove);
    std::optional<Move> checkImmediateWin(SparseBoard& board, Player player);
    std::optional<Move> checkImmediateBlock(SparseBoard& board, Player player);
    std::optional<Move> checkDangerousThreat(SparseBoard& board, Player player);
    int evaluateTerminal(const SparseBoard& board, Player player);
    bool hasThreats(const SparseBoard& board, Player player);
};

} // namespace tictactoe

