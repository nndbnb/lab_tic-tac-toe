#pragma once

#include "board/sparse_board.h"
#include "engine/evaluator.h"
#include "engine/config.h"
#include "adt/sequence.h"
#include <unordered_set>

namespace tictactoe {

struct Move {
    int x, y;
    int score;
    
    Move(int x = 0, int y = 0, int score = 0) : x(x), y(y), score(score) {}
    
    bool operator<(const Move& other) const {
        return score > other.score;
    }
    
    bool operator==(const Move& other) const {
        return x == other.x && y == other.y;
    }
};

class MoveGenerator {
public:
    explicit MoveGenerator(int win_length);
    
    adt::ArraySequence<Move> generateCandidates(const SparseBoard& board, Player player);
    int scoreMove(const SparseBoard& board, int x, int y, Player player);
    void sortAndPrune(adt::ArraySequence<Move>& moves, int topK);
    std::optional<Move> checkImmediateWin(const SparseBoard& board, Player player);
    std::optional<Move> checkImmediateBlock(const SparseBoard& board, Player player);
    std::optional<Move> checkDangerousThreat(const SparseBoard& board, Player player);
    
private:
    Evaluator evaluator_;
    int win_length_;
    
    adt::ArraySequence<Position> generateRadiusCandidates(
        const SparseBoard& board, int radius);
    void addNeighbors(int x, int y, int radius, 
                     std::unordered_set<Position, PositionHash>& candidates,
                     const SparseBoard& board);
};

} // namespace tictactoe

