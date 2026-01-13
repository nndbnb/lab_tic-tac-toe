#pragma once

#include "board/sparse_board.h"
#include "engine/config.h"
#include "adt/sequence.h"
#include <array>

namespace tictactoe {

struct Pattern {
    int length;
    bool isOpen;
    bool isBroken;
    int score;
};

class Evaluator {
public:
    explicit Evaluator(int win_length);
    
    int evaluatePosition(const SparseBoard& board, Player player);
    int evaluateMove(const SparseBoard& board, int x, int y, Player player);
    adt::ArraySequence<Pattern> detectPatterns(const SparseBoard& board, int x, int y, Player player);
    int getPatternScore(int length, bool isOpen) const;
    void initPatternWeights(int N);
    int getWinLength() const { return win_length_; }
    
private:
    int win_length_;
    std::array<int, 20> open_pattern_scores_;
    std::array<int, 20> closed_pattern_scores_;
    
    static const Position directions_[4];
    
    Pattern analyzeLine(const SparseBoard& board, int x, int y, const Position& dir, Player player);
    
    struct LineInfo {
        int own_count;
        int left_space;
        int right_space;
        bool has_break;
    };
    
    LineInfo analyzeLineInfo(const SparseBoard& board, int x, int y, const Position& dir, Player player);
    int calculatePatternScore(int length, bool isOpen, bool isBroken) const;
    int detectForks(const SparseBoard& board, int x, int y, Player player);
};

} // namespace tictactoe

