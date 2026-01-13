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

struct SearchStats {
    int nodes_searched;
    int depth_reached;
    int time_ms;
    Move principal_variation[20];
    int pv_length;
    DecisionType decision_type;
    int final_score;
    
    SearchStats() : nodes_searched(0), depth_reached(0), time_ms(0), pv_length(0),
                    decision_type(DecisionType::NEGAMAX_SEARCH), final_score(0) {
        for (int i = 0; i < 20; ++i) {
            principal_variation[i] = Move(0, 0);
        }
    }
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

