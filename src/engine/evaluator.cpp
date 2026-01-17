#include "engine/evaluator.h"
#include "board/sparse_board.h"
#include "engine/config.h"
#include <cmath>
#include <algorithm>
#include <limits>

namespace tictactoe {

const Position Evaluator::directions_[4] = {
    Position(1, 0),
    Position(0, 1),
    Position(1, 1),
    Position(1, -1)
};

Evaluator::Evaluator(int win_length) : win_length_(win_length) {
    initPatternWeights(win_length);
}

void Evaluator::initPatternWeights(int N) {
    win_length_ = N;
    
    for (int i = 0; i < 20; ++i) {
        open_pattern_scores_[i] = 0;
        closed_pattern_scores_[i] = 0;
    }
    
    for (int k = 1; k < N && k < 20; ++k) {
        double baseScore = std::pow(10.0, k);
        double proximityBonus = std::pow(4.0, N - k);
        open_pattern_scores_[k] = static_cast<int>(baseScore * proximityBonus * 2.0);
        closed_pattern_scores_[k] = static_cast<int>(baseScore * proximityBonus);
    }
}

int Evaluator::getPatternScore(int length, bool isOpen) const {
    if (length <= 0 || length >= win_length_) {
        return 0;
    }
    
    if (isOpen) {
        return open_pattern_scores_[length];
    } else {
        return closed_pattern_scores_[length];
    }
}

int Evaluator::calculatePatternScore(int length, bool isOpen, bool isBroken) const {
    int baseScore = getPatternScore(length, isOpen);
    if (isBroken) {
        return baseScore / 2;
    }
    return baseScore;
}

Evaluator::LineInfo Evaluator::analyzeLineInfo(
    const SparseBoard& board, int x, int y, const Position& dir, Player player) {
    
    LineInfo info = {0, 0, 0, false};
    Position current(x, y);
    int consecutive = 0;
    bool foundBreak = false;
    
    if (board.at(current.x, current.y) == player) {
        consecutive = 1;
    }
    
    current = Position(x, y) + dir;
    int maxIterations = 20;
    int iterations = 0;
    while (iterations < maxIterations) {
        iterations++;
        Player cell = board.at(current.x, current.y);
        if (cell == player) {
            consecutive++;
            current = current + dir;
        } else if (cell == Player::None) {
            info.right_space++;
            if (consecutive > 0 && info.right_space == 1) {
                Position next = current + dir;
                if (board.at(next.x, next.y) == player) {
                    foundBreak = true;
                }
            }
            current = current + dir;
        } else {
            break;
        }
    }
    
    current = Position(x, y) - dir;
    iterations = 0;
    while (iterations < maxIterations) {
        iterations++;
        Player cell = board.at(current.x, current.y);
        if (cell == player) {
            consecutive++;
            current = current - dir;
        } else if (cell == Player::None) {
            info.left_space++;
            if (consecutive > 0 && info.left_space == 1) {
                Position prev = current - dir;
                if (board.at(prev.x, prev.y) == player) {
                    foundBreak = true;
                }
            }
            current = current - dir;
        } else {
            break;
        }
    }
    
    info.own_count = consecutive;
    info.has_break = foundBreak;
    
    return info;
}

Pattern Evaluator::analyzeLine(
    const SparseBoard& board, int x, int y, const Position& dir, Player player) {
    
    LineInfo info = analyzeLineInfo(board, x, y, dir, player);
    
    Pattern pattern;
    pattern.length_ = info.own_count;
    pattern.isOpen_ = (info.left_space > 0 && info.right_space > 0);
    pattern.isBroken_ = info.has_break;
    pattern.score_ = calculatePatternScore(pattern.length_, pattern.isOpen_, pattern.isBroken_);
    
    return pattern;
}

adt::ArraySequence<Pattern> Evaluator::detectPatterns(
    const SparseBoard& board, int x, int y, Player player) {
    
    adt::ArraySequence<Pattern> patterns;
    
    for (int i = 0; i < 4; ++i) {
        Pattern p = analyzeLine(board, x, y, directions_[i], player);
        if (p.getLength() > 0) {
            patterns.AppendInPlace(p);
        }
    }
    
    return patterns;
}

int Evaluator::detectForks(const SparseBoard& board, int x, int y, Player player) {
    auto patterns = detectPatterns(board, x, y, player);
    
    int threatCount = 0;
    int totalScore = 0;
    
    for (int i = 0; i < patterns.GetLength(); ++i) {
        const auto& pattern = patterns.Get(i);
        if (pattern.getLength() >= win_length_ - 1 && pattern.isOpen()) {
            threatCount++;
        }
        totalScore += pattern.getScore();
    }
    
    if (threatCount >= 2) {
        return totalScore + Config::FORK_BONUS;
    }
    
    return totalScore;
}

int Evaluator::evaluateMove(const SparseBoard& board, int x, int y, Player player) {
    SparseBoard testBoard = board;
    
    if (testBoard.makeMove(x, y, player)) {
        if (testBoard.isWin(x, y, player)) {
            return std::numeric_limits<int>::max() / 2;
        }
    }
    
    int score = detectForks(board, x, y, player);
    
    Player opponent = (player == Player::X) ? Player::O : Player::X;
    auto opponentPatterns = detectPatterns(board, x, y, opponent);
    
    for (int i = 0; i < opponentPatterns.GetLength(); ++i) {
        const auto& pattern = opponentPatterns.Get(i);
        if (pattern.getLength() >= win_length_ - 1) {
            score += pattern.getScore();
        }
    }
    
    return score;
}

int Evaluator::evaluatePosition(const SparseBoard& board, Player player) {
    int score = 0;
    auto occupied = board.getOccupiedPositions();
    
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        Player cellPlayer = board.at(pos.x, pos.y);
        
        if (cellPlayer == player) {
            auto patterns = detectPatterns(board, pos.x, pos.y, player);
            for (int j = 0; j < patterns.GetLength(); ++j) {
                score += patterns.Get(j).getScore();
            }
        } else {
            auto patterns = detectPatterns(board, pos.x, pos.y, cellPlayer);
            for (int j = 0; j < patterns.GetLength(); ++j) {
                score -= patterns.Get(j).getScore();
            }
        }
    }
    
    return score;
}

} // namespace tictactoe

