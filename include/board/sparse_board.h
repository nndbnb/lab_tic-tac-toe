#pragma once

#include <unordered_map>
#include <optional>
#include <cstdint>
#include <algorithm>
#include "adt/sequence.h"

namespace tictactoe {

enum class Player {
    None = 0,
    X = 1,
    O = 2,
};

struct Position {
    int x, y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
    
    Position operator+(const Position& other) const {
        return Position(x + other.x, y + other.y);
    }
    
    Position operator-(const Position& other) const {
        return Position(x - other.x, y - other.y);
    }
};

struct PositionHash {
    std::size_t operator()(const Position& pos) const {
        return std::hash<int>()(pos.x) ^ (std::hash<int>()(pos.y) << 1);
    }
};

class BoundingBox {
public:
    BoundingBox() : min_x_(0), max_x_(0), min_y_(0), max_y_(0) {}
    
    int getMinX() const { return min_x_; }
    int getMaxX() const { return max_x_; }
    int getMinY() const { return min_y_; }
    int getMaxY() const { return max_y_; }
    int getWidth() const { return max_x_ - min_x_ + 1; }
    int getHeight() const { return max_y_ - min_y_ + 1; }
    
    void expand(int x, int y) {
        if (min_x_ == max_x_ && min_y_ == max_y_ && min_x_ == 0 && min_y_ == 0) {
            // First expansion
            min_x_ = max_x_ = x;
            min_y_ = max_y_ = y;
        } else {
            min_x_ = std::min(min_x_, x);
            max_x_ = std::max(max_x_, x);
            min_y_ = std::min(min_y_, y);
            max_y_ = std::max(max_y_, y);
        }
    }
    
    friend class SparseBoard;
    
private:
    int min_x_, max_x_, min_y_, max_y_;
};

class SparseBoard {
public:
    explicit SparseBoard(int win_length = 5);
    
    SparseBoard(const SparseBoard& other);
    
    SparseBoard& operator=(const SparseBoard& other);
    
    bool makeMove(int x, int y, Player player);
    void undoMove(int x, int y);
    
    Player at(int x, int y) const;
    bool isEmpty(int x, int y) const;
    bool isWin(int x, int y, Player player) const;
    bool isTerminal() const;
    
    int getWinLength() const { return win_length_; }
    
    BoundingBox getBoundingBox() const { return bbox_; }
    
    adt::ArraySequence<Position> getOccupiedPositions() const;
    
    uint64_t getZobristHash() const { return zobrist_hash_; }
    
    struct Move {
        int x, y;
        Player player;
    };
    
    adt::ArraySequence<Move> getMoveHistory() const { return move_history_; }
    
private:
    int win_length_;
    std::unordered_map<Position, Player, PositionHash> cells_;
    BoundingBox bbox_;
    uint64_t zobrist_hash_;
    adt::ArraySequence<Move> move_history_;
    
    static const Position directions_[4];
    
    void updateZobristHash(int x, int y, Player player);
    
    int countInDirection(int x, int y, const Position& dir, Player player) const;
    bool checkWinInDirection(int x, int y, const Position& dir, Player player) const;
};

} // namespace tictactoe

