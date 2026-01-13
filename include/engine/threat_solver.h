#pragma once

#include "board/sparse_board.h"
#include "engine/move_generator.h"
#include "engine/config.h"
#include "adt/sequence.h"
#include <optional>

namespace tictactoe {

class ThreatSolver {
public:
    explicit ThreatSolver(int win_length);
    
    std::optional<Move> findForcedWin(SparseBoard& board, Player player, int maxDepth);
    
private:
    MoveGenerator moveGen_;
    int win_length_;
    
    adt::ArraySequence<Move> generateThreats(const SparseBoard& board, Player player);
    adt::ArraySequence<Move> findDefensiveMoves(const SparseBoard& board, Player player);
    bool searchForcedWin(SparseBoard& board, Player player, int depth, int maxDepth);
    bool isDirectThreat(const SparseBoard& board, int x, int y, Player player);
};

} // namespace tictactoe

