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

struct BoundingBox {
    int min_x, max_x, min_y, max_y;
    
    BoundingBox() : min_x(0), max_x(0), min_y(0), max_y(0) {}
    
    void expand(int x, int y) {
        if (min_x == max_x && min_y == max_y && min_x == 0 && min_y == 0) {
            // First expansion
            min_x = max_x = x;
            min_y = max_y = y;
        } else {
            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
    }
    
    int width() const { return max_x - min_x + 1; }
    int height() const { return max_y - min_y + 1; }
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
    void updateZobristHash(int x, int y, Player player);
    
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
    
    int countInDirection(int x, int y, const Position& dir, Player player) const;
    bool checkWinInDirection(int x, int y, const Position& dir, Player player) const;
};

} // namespace tictactoe

