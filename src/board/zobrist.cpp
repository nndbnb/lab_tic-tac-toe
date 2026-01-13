#include "board/zobrist.h"
#include <algorithm>

namespace tictactoe {

ZobristHasher::ZobristHasher() {
    initialize();
}

void ZobristHasher::initialize(uint64_t seed) {
    if (seed == 0) {
        std::random_device rd;
        seed = rd();
        seed = (seed << 32) | rd();
    }
    rng_.seed(seed);
    keys_.clear();
}

uint64_t ZobristHasher::generateKey(int x, int y, Player player) const {
    int index = coordToIndex(x, y);
    int playerIndex = static_cast<int>(player);
    
    uint64_t combined = (static_cast<uint64_t>(index) << 2) | playerIndex;
    
    std::mt19937_64 local_rng(combined);
    return local_rng();
}

uint64_t ZobristHasher::getKey(int x, int y, Player player) {
    if (player == Player::None) {
        return 0;
    }
    
    int index = coordToIndex(x, y);
    int playerIndex = static_cast<int>(player);
    
    uint64_t baseKey = generateKey(x, y, player);
    
    uint64_t cacheKey = (static_cast<uint64_t>(index) << 2) | playerIndex;
    auto it = keys_.find(cacheKey);
    if (it != keys_.end()) {
        return it->second;
    }
    
    keys_[cacheKey] = baseKey;
    return baseKey;
}

} // namespace tictactoe

