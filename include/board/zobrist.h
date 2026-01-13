#pragma once

#include <cstdint>
#include <random>
#include <unordered_map>
#include "sparse_board.h"

namespace tictactoe {

class ZobristHasher {
public:
    ZobristHasher();
    
    uint64_t getKey(int x, int y, Player player);
    
    void initialize(uint64_t seed = 0);
    
private:
    static constexpr int COORD_RANGE = 2001;
    static constexpr int COORD_OFFSET = 1000;
    
    std::unordered_map<int, uint64_t> keys_;
    std::mt19937_64 rng_;
    
    int coordToIndex(int x, int y) const {
        return (x + COORD_OFFSET) * COORD_RANGE + (y + COORD_OFFSET);
    }
    
    uint64_t generateKey(int x, int y, Player player) const;
};

} // namespace tictactoe

