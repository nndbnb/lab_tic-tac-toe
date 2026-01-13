#pragma once

#include <cstdint>
#include <optional>
#include "board/sparse_board.h"
#include "engine/config.h"
#include "engine/move_generator.h"

namespace tictactoe {

// Forward declaration
struct Move;

enum class TTFlag {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
};

struct TTEntry {
    uint64_t zobristKey;
    int16_t score;
    int8_t depth;
    int8_t flag;
    Move bestMove;
    uint32_t age;
    
    TTEntry() : zobristKey(0), score(0), depth(0), flag(0), bestMove(0, 0), age(0) {}
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t sizeMB = Config::TT_SIZE_MB);
    ~TranspositionTable();
    
    struct ProbeResult {
        bool found;
        int score;
        Move bestMove;
    };
    
    ProbeResult probe(uint64_t key, int depth, int alpha, int beta);
    
    void store(uint64_t key, int score, int depth, TTFlag flag, Move bestMove);
    
    void clear();
    
    std::optional<Move> getPVMove(uint64_t key);
    
    void incrementAge() { age_++; }
    
    size_t getSize() const { return size_; }
    size_t getEntries() const { return entries_; }
    
private:
    size_t size_;
    size_t entries_;
    TTEntry* table_;
    uint32_t age_;
    
    size_t index(uint64_t key) const {
        return key % size_;
    }
    
    void replaceEntry(size_t idx, uint64_t key, int score, int depth, 
                     TTFlag flag, Move bestMove);
};

} // namespace tictactoe

