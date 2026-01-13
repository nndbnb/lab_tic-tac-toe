#include "engine/transposition_table.h"
#include "engine/config.h"
#include <cstring>
#include <algorithm>

namespace tictactoe {

TranspositionTable::TranspositionTable(size_t sizeMB) 
    : entries_(0), age_(0) {
    
    size_t targetSize = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
    
    size_ = 1;
    while (size_ < targetSize && size_ < (1ULL << 30)) {
        size_ <<= 1;
    }
    size_ >>= 1;
    
    table_ = new TTEntry[size_];
    clear();
}

TranspositionTable::~TranspositionTable() {
    delete[] table_;
}

void TranspositionTable::clear() {
    for (size_t i = 0; i < size_; ++i) {
        table_[i] = TTEntry();
    }
    entries_ = 0;
    age_ = 0;
}

TranspositionTable::ProbeResult TranspositionTable::probe(
    uint64_t key, int depth, int alpha, int beta) {
    
    size_t idx = index(key);
    TTEntry& entry = table_[idx];
    
    ProbeResult result = {false, 0, Move(0, 0)};
    
    if (entry.zobristKey == key && entry.depth >= depth) {
        result.found = true;
        result.bestMove = entry.bestMove;
        
        if (entry.flag == static_cast<int8_t>(TTFlag::EXACT)) {
            result.score = entry.score;
        } else if (entry.flag == static_cast<int8_t>(TTFlag::LOWER_BOUND)) {
            if (entry.score >= beta) {
                result.score = entry.score;
            } else {
                result.found = false;
            }
        } else if (entry.flag == static_cast<int8_t>(TTFlag::UPPER_BOUND)) {
            if (entry.score <= alpha) {
                result.score = entry.score;
            } else {
                result.found = false;
            }
        }
    }
    
    return result;
}

void TranspositionTable::store(uint64_t key, int score, int depth, 
                               TTFlag flag, Move bestMove) {
    size_t idx = index(key);
    TTEntry& entry = table_[idx];
    
    bool isEmpty = (entry.zobristKey == 0 && entry.depth == 0);
    if (isEmpty || entry.depth <= depth) {
        bool wasEmpty = isEmpty;
        replaceEntry(idx, key, score, depth, flag, bestMove);
        if (wasEmpty) {
            entries_++;
        }
    }
}

void TranspositionTable::replaceEntry(size_t idx, uint64_t key, int score, 
                                     int depth, TTFlag flag, Move bestMove) {
    TTEntry& entry = table_[idx];
    entry.zobristKey = key;
    entry.score = static_cast<int16_t>(score);
    entry.depth = static_cast<int8_t>(depth);
    entry.flag = static_cast<int8_t>(flag);
    entry.bestMove = bestMove;
    entry.age = age_;
}

std::optional<Move> TranspositionTable::getPVMove(uint64_t key) {
    size_t idx = index(key);
    TTEntry& entry = table_[idx];
    
    if (entry.zobristKey == key && entry.bestMove.x != 0 && entry.bestMove.y != 0) {
        return entry.bestMove;
    }
    
    return std::nullopt;
}

} // namespace tictactoe

