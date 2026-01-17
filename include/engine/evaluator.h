#pragma once

#include "board/sparse_board.h"
#include "engine/config.h"
#include "adt/sequence.h"
#include <array>

namespace tictactoe {

class Pattern {
public:
    Pattern() : length_(0), isOpen_(false), isBroken_(false), score_(0) {}
    Pattern(int length, bool isOpen, bool isBroken, int score) 
        : length_(length), isOpen_(isOpen), isBroken_(isBroken), score_(score) {}
    
    int getLength() const { return length_; }
    bool isOpen() const { return isOpen_; }
    bool isBroken() const { return isBroken_; }
    int getScore() const { return score_; }
    
    friend class Evaluator;
    
private:
    int length_;
    bool isOpen_;
    bool isBroken_;
    int score_;
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

