#include "engine/threat_solver.h"
#include "engine/evaluator.h"
#include "board/sparse_board.h"
#include <algorithm>

namespace tictactoe {

ThreatSolver::ThreatSolver(int win_length) 
    : moveGen_(win_length), win_length_(win_length) {
}

bool ThreatSolver::isDirectThreat(const SparseBoard& board, int x, int y, Player player) {
    SparseBoard testBoard = board;
    if (!testBoard.makeMove(x, y, player)) {
        return false;
    }
    
    const Position directions[4] = {
        Position(1, 0), Position(0, 1), Position(1, 1), Position(1, -1)
    };
    
    for (int i = 0; i < 4; ++i) {
        int count = 1;
        
        Position current(x, y);
        current = current + directions[i];
        int rightSpace = 0;
        int maxIterations = 20;
        int iterations = 0;
        while (iterations < maxIterations && testBoard.at(current.x, current.y) == player) {
            count++;
            current = current + directions[i];
            iterations++;
        }
        if (testBoard.at(current.x, current.y) == Player::None) {
            rightSpace = 1;
        }
        
        current = Position(x, y) - directions[i];
        int leftSpace = 0;
        iterations = 0;
        while (iterations < maxIterations && testBoard.at(current.x, current.y) == player) {
            count++;
            current = current - directions[i];
            iterations++;
        }
        if (testBoard.at(current.x, current.y) == Player::None) {
            leftSpace = 1;
        }
        
        if (count == win_length_ - 1 && leftSpace > 0 && rightSpace > 0) {
            return true;
        }
    }
    
    return false;
}

adt::ArraySequence<Move> ThreatSolver::generateThreats(
    const SparseBoard& board, Player player) {
    
    adt::ArraySequence<Move> threats;
    auto candidates = moveGen_.generateCandidates(board, player);
    
    for (int i = 0; i < candidates.GetLength(); ++i) {
        const auto& move = candidates.Get(i);
        if (isDirectThreat(board, move.x, move.y, player)) {
            threats.AppendInPlace(move);
        }
    }
    
    return threats;
}

adt::ArraySequence<Move> ThreatSolver::findDefensiveMoves(
    const SparseBoard& board, Player player) {
    
    Player opponent = (player == Player::X) ? Player::O : Player::X;
    
    adt::ArraySequence<Move> defenses;
    auto opponentThreats = generateThreats(board, opponent);
    
    for (int i = 0; i < opponentThreats.GetLength(); ++i) {
        const auto& threat = opponentThreats.Get(i);
        defenses.AppendInPlace(Move(threat.x, threat.y, threat.score));
    }
    
    auto blockMove = moveGen_.checkImmediateBlock(board, player);
    if (blockMove.has_value()) {
        defenses.AppendInPlace(*blockMove);
    }
    
    return defenses;
}

bool ThreatSolver::searchForcedWin(
    SparseBoard& board, Player player, int depth, int maxDepth) {
    
    if (depth >= maxDepth) {
        return false;
    }
    
    auto winMove = moveGen_.checkImmediateWin(board, player);
    if (winMove.has_value()) {
        return true;
    }
    
    auto threats = generateThreats(board, player);
    
    for (int i = 0; i < threats.GetLength(); ++i) {
        const auto& threat = threats.Get(i);
        board.makeMove(threat.x, threat.y, player);
        
        Player opponent = (player == Player::X) ? Player::O : Player::X;
        auto defenses = findDefensiveMoves(board, opponent);
        
        if (defenses.Empty()) {
            board.undoMove(threat.x, threat.y);
            return true;
        }
        
        bool allDefensesFail = true;
        for (int j = 0; j < defenses.GetLength(); ++j) {
            const auto& defense = defenses.Get(j);
            board.makeMove(defense.x, defense.y, opponent);
            
            if (!searchForcedWin(board, player, depth + 1, maxDepth)) {
                allDefensesFail = false;
            }
            
            board.undoMove(defense.x, defense.y);
            
            if (!allDefensesFail) break;
        }
        
        board.undoMove(threat.x, threat.y);
        
        if (allDefensesFail) {
            return true;
        }
    }
    
    return false;
}

std::optional<Move> ThreatSolver::findForcedWin(
    SparseBoard& board, Player player, int maxDepth) {
    
    auto winMove = moveGen_.checkImmediateWin(board, player);
    if (winMove.has_value()) {
        return *winMove;
    }
    
    auto threats = generateThreats(board, player);
    
    for (int i = 0; i < threats.GetLength(); ++i) {
        const auto& threat = threats.Get(i);
        board.makeMove(threat.x, threat.y, player);
        
        Player opponent = (player == Player::X) ? Player::O : Player::X;
        auto defenses = findDefensiveMoves(board, opponent);
        
        bool allDefensesFail = true;
        for (int j = 0; j < defenses.GetLength(); ++j) {
            const auto& defense = defenses.Get(j);
            board.makeMove(defense.x, defense.y, opponent);
            
            if (!searchForcedWin(board, player, 1, maxDepth)) {
                allDefensesFail = false;
            }
            
            board.undoMove(defense.x, defense.y);
            
            if (!allDefensesFail) break;
        }
        
        board.undoMove(threat.x, threat.y);
        
        if (allDefensesFail) {
            return threat;
        }
    }
    
    return std::nullopt;
}

} // namespace tictactoe

