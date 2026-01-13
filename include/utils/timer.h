#pragma once

#include <chrono>

namespace tictactoe {

class Timer {
public:
    Timer() : start_time_(std::chrono::steady_clock::now()) {}
    
    void reset() {
        start_time_ = std::chrono::steady_clock::now();
    }
    
    int elapsedMs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time_).count();
    }
    
    bool isTimeout(int timeLimitMs) const {
        return elapsedMs() >= timeLimitMs;
    }
    
private:
    std::chrono::steady_clock::time_point start_time_;
};

} // namespace tictactoe

