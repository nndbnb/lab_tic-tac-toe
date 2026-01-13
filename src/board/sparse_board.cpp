#include "board/sparse_board.h"
#include "board/zobrist.h"
#include <algorithm>
#include <cmath>

namespace tictactoe {

namespace {
    ZobristHasher g_zobrist_hasher;
}

const Position SparseBoard::directions_[4] = {
    Position(1, 0),
    Position(0, 1),
    Position(1, 1),
    Position(1, -1)
};

SparseBoard::SparseBoard(int win_length) 
    : win_length_(win_length), zobrist_hash_(0) {
}

SparseBoard::SparseBoard(const SparseBoard& other)
    : win_length_(other.win_length_),
      cells_(other.cells_),
      bbox_(other.bbox_),
      zobrist_hash_(other.zobrist_hash_),
      move_history_(other.move_history_) {
}

SparseBoard& SparseBoard::operator=(const SparseBoard& other) {
    if (this != &other) {
        win_length_ = other.win_length_;
        cells_ = other.cells_;
        bbox_ = other.bbox_;
        zobrist_hash_ = other.zobrist_hash_;
        move_history_ = other.move_history_;
    }
    return *this;
}

bool SparseBoard::makeMove(int x, int y, Player player) {
    Position pos(x, y);
    
    if (cells_.find(pos) != cells_.end()) {
        return false;
    }
    
    if (player == Player::None) {
        return false;
    }
    
    cells_[pos] = player;
    bbox_.expand(x, y);
    
    updateZobristHash(x, y, player);
    
    move_history_.AppendInPlace({x, y, player});
    
    return true;
}

void SparseBoard::undoMove(int x, int y) {
    Position pos(x, y);
    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        Player player = it->second;
        cells_.erase(it);
        
        updateZobristHash(x, y, player);
        
        if (!move_history_.Empty() && 
            move_history_.Back().x == x && 
            move_history_.Back().y == y) {
            move_history_.PopBack();
        }
    }
}

Player SparseBoard::at(int x, int y) const {
    Position pos(x, y);
    auto it = cells_.find(pos);
    if (it != cells_.end()) {
        return it->second;
    }
    return Player::None;
}

bool SparseBoard::isEmpty(int x, int y) const {
    return at(x, y) == Player::None;
}

int SparseBoard::countInDirection(int x, int y, const Position& dir, Player player) const {
    int count = 0;
    Position pos(x, y);
    
    Position current = pos + dir;
    while (cells_.find(current) != cells_.end() && 
           cells_.at(current) == player) {
        count++;
        current = current + dir;
    }
    
    current = pos - dir;
    while (cells_.find(current) != cells_.end() && 
           cells_.at(current) == player) {
        count++;
        current = current - dir;
    }
    
    return count;
}

bool SparseBoard::checkWinInDirection(int x, int y, const Position& dir, Player player) const {
    Position pos(x, y);
    if (cells_.find(pos) == cells_.end() || cells_.at(pos) != player) {
        return false;
    }
    
    int count = 1;
    
    Position current = pos + dir;
    int maxIterations = 20;
    int iterations = 0;
    while (iterations < maxIterations && 
           cells_.find(current) != cells_.end() && 
           cells_.at(current) == player) {
        count++;
        current = current + dir;
        iterations++;
    }
    
    current = pos - dir;
    iterations = 0;
    while (iterations < maxIterations && 
           cells_.find(current) != cells_.end() && 
           cells_.at(current) == player) {
        count++;
        current = current - dir;
        iterations++;
    }
    
    return count >= win_length_;
}

bool SparseBoard::isWin(int x, int y, Player player) const {
    for (int i = 0; i < 4; ++i) {
        if (checkWinInDirection(x, y, directions_[i], player)) {
            return true;
        }
    }
    return false;
}

bool SparseBoard::isTerminal() const {
    for (const auto& [pos, player] : cells_) {
        if (isWin(pos.x, pos.y, player)) {
            return true;
        }
    }
    return false;
}

void SparseBoard::updateZobristHash(int x, int y, Player player) {
    uint64_t key = g_zobrist_hasher.getKey(x, y, player);
    zobrist_hash_ ^= key;
}

adt::ArraySequence<Position> SparseBoard::getOccupiedPositions() const {
    adt::ArraySequence<Position> positions;
    positions.Reserve(cells_.size());
    for (const auto& [pos, player] : cells_) {
        positions.AppendInPlace(pos);
    }
    return positions;
}

} // namespace tictactoe

