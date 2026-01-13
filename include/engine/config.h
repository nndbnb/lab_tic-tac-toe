#pragma once

namespace tictactoe {

namespace Config {
    inline int WIN_LENGTH = 5;
    inline int CANDIDATE_RADIUS = 2;
    inline int TOP_K_CANDIDATES = 30;
    inline int MAX_DEPTH = 12;
    inline int TT_SIZE_MB = 128;
    inline int DEFAULT_TIME_MS = 5000;
    inline int THREAT_SOLVER_MAX_DEPTH = 4;
    inline int FORK_BONUS = 5000;
    inline int STABLE_ITERATIONS_THRESHOLD = 2;
    inline int STABLE_SCORE_THRESHOLD = 50;
}

} // namespace tictactoe

