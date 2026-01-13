#include "engine/move_generator.h"
#include "board/sparse_board.h"
#include "engine/config.h"
#include <algorithm>
#include <optional>
#include <limits>
#include <unordered_set>
#include <unordered_map>

namespace tictactoe {

MoveGenerator::MoveGenerator(int win_length) 
    : evaluator_(win_length), win_length_(win_length) {
}

std::optional<Move> MoveGenerator::checkImmediateWin(
    const SparseBoard& board, Player player) {
    
    auto occupied = board.getOccupiedPositions();
    
    if (occupied.Empty()) {
        return std::nullopt;
    }
    
    std::unordered_set<Position, PositionHash> candidateSet;
    
    const Position directions[4] = {
        Position(1, 0), Position(0, 1), Position(1, 1), Position(1, -1)
    };
    
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        if (board.at(pos.x, pos.y) != player) continue;
        
        for (int d = 0; d < 4; ++d) {
            int count = 1;
            Position current = pos;
            
            Position forward = current + directions[d];
            int maxIter = win_length_;
            int iter = 0;
            while (iter < maxIter && board.at(forward.x, forward.y) == player) {
                count++;
                forward = forward + directions[d];
                iter++;
            }
            
            Position backward = current - directions[d];
            iter = 0;
            while (iter < maxIter && board.at(backward.x, backward.y) == player) {
                count++;
                backward = backward - directions[d];
                iter++;
            }
            
            if (count >= win_length_ - 1) {
                if (board.isEmpty(forward.x, forward.y)) {
                    candidateSet.insert(forward);
                }
                if (board.isEmpty(backward.x, backward.y)) {
                    candidateSet.insert(backward);
                }
            }
        }
    }
    
    for (const auto& pos : candidateSet) {
        for (int d = 0; d < 4; ++d) {
            int lineCount = 1;
            Position forward = pos + directions[d];
            int maxIter = win_length_;
            int iter = 0;
            while (iter < maxIter && board.at(forward.x, forward.y) == player) {
                lineCount++;
                forward = forward + directions[d];
                iter++;
            }
            Position backward = pos - directions[d];
            iter = 0;
            while (iter < maxIter && board.at(backward.x, backward.y) == player) {
                lineCount++;
                backward = backward - directions[d];
                iter++;
            }
            if (lineCount >= win_length_) {
                return Move(pos.x, pos.y, std::numeric_limits<int>::max());
            }
        }
    }
    
    return std::nullopt;
}

std::optional<Move> MoveGenerator::checkImmediateBlock(
    const SparseBoard& board, Player player) {
    
    Player opponent = (player == Player::X) ? Player::O : Player::X;
    
    auto winMove = checkImmediateWin(board, opponent);
    if (winMove.has_value()) {
        return Move(winMove->x, winMove->y, std::numeric_limits<int>::max() - 1);
    }
    
    return std::nullopt;
}

std::optional<Move> MoveGenerator::checkDangerousThreat(
    const SparseBoard& board, Player player) {
    
    if (win_length_ < 4) {
        return std::nullopt;
    }
    
    int threatLength = win_length_ - 2;
    Player opponent = (player == Player::X) ? Player::O : Player::X;
    
    auto occupied = board.getOccupiedPositions();
    if (occupied.Empty()) {
        return std::nullopt;
    }
    
    std::unordered_set<Position, PositionHash> blockingMoves;
    
    const Position directions[4] = {
        Position(1, 0), Position(0, 1), Position(1, 1), Position(1, -1)
    };
    
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        if (board.at(pos.x, pos.y) != opponent) continue;
        
        for (int d = 0; d < 4; ++d) {
            int count = 1;
            Position current = pos;
            
            Position forward = current + directions[d];
            int maxIter = win_length_;
            int iter = 0;
            while (iter < maxIter && board.at(forward.x, forward.y) == opponent) {
                count++;
                forward = forward + directions[d];
                iter++;
            }
            
            Position backward = current - directions[d];
            iter = 0;
            while (iter < maxIter && board.at(backward.x, backward.y) == opponent) {
                count++;
                backward = backward - directions[d];
                iter++;
            }
            
            if (count == threatLength) {
                bool leftOpen = board.isEmpty(backward.x, backward.y);
                bool rightOpen = board.isEmpty(forward.x, forward.y);
                
                bool leftValid = leftOpen;
                bool rightValid = rightOpen;
                
                if (leftOpen) {
                    Position nextLeft = backward - directions[d];
                    if (board.at(nextLeft.x, nextLeft.y) == opponent) {
                        leftValid = false;
                    }
                }
                
                if (rightOpen) {
                    Position nextRight = forward + directions[d];
                    if (board.at(nextRight.x, nextRight.y) == opponent) {
                        rightValid = false;
                    }
                }
                
                if (leftValid && rightValid) {
                    if (leftValid) {
                        blockingMoves.insert(backward);
                    }
                    if (rightValid) {
                        blockingMoves.insert(forward);
                    }
                }
            }
        }
    }
    
    if (!blockingMoves.empty()) {
        const auto& blockPos = *blockingMoves.begin();
        return Move(blockPos.x, blockPos.y, std::numeric_limits<int>::max() - 2);
    }
    
    return std::nullopt;
}

void MoveGenerator::addNeighbors(int x, int y, int radius,
                                std::unordered_set<Position, PositionHash>& candidates,
                                const SparseBoard& board) {
    for (int dx = -radius; dx <= radius; ++dx) {
        for (int dy = -radius; dy <= radius; ++dy) {
            if (dx == 0 && dy == 0) continue;
            
            int nx = x + dx;
            int ny = y + dy;
            
            if (board.isEmpty(nx, ny)) {
                candidates.insert(Position(nx, ny));
            }
        }
    }
}

adt::ArraySequence<Position> MoveGenerator::generateRadiusCandidates(
    const SparseBoard& board, int radius) {
    
    std::unordered_set<Position, PositionHash> candidateSet;
    auto occupied = board.getOccupiedPositions();
    
    if (occupied.Empty()) {
        adt::ArraySequence<Position> result;
        result.AppendInPlace(Position(0, 0));
        return result;
    }
    
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        addNeighbors(pos.x, pos.y, radius, candidateSet, board);
    }
    
    adt::ArraySequence<Position> candidates;
    for (const auto& pos : candidateSet) {
        candidates.AppendInPlace(pos);
    }
    return candidates;
}

int MoveGenerator::scoreMove(const SparseBoard& board, int x, int y, Player player) {
    return evaluator_.evaluateMove(board, x, y, player);
}

void MoveGenerator::sortAndPrune(adt::ArraySequence<Move>& moves, int topK) {
    moves.SortInPlace();
    if (moves.GetLength() > topK) {
        moves.Resize(topK);
    }
}

adt::ArraySequence<Move> MoveGenerator::generateCandidates(
    const SparseBoard& board, Player player) {
    
    adt::ArraySequence<Move> candidates;
    
    auto winMove = checkImmediateWin(board, player);
    if (winMove.has_value()) {
        adt::ArraySequence<Move> result;
        result.AppendInPlace(*winMove);
        return result;
    }
    
    auto blockMove = checkImmediateBlock(board, player);
    if (blockMove.has_value()) {
        candidates.AppendInPlace(*blockMove);
    }
    
    auto positions = generateRadiusCandidates(board, Config::CANDIDATE_RADIUS);
    
    if (positions.GetLength() == 1 && positions.Get(0).x == 0 && positions.Get(0).y == 0) {
        adt::ArraySequence<Move> result;
        result.AppendInPlace(Move(0, 0, 0));
        return result;
    }
    
    int scoreLimit = Config::TOP_K_CANDIDATES * 2;
    int scored = 0;
    
    for (int i = 0; i < positions.GetLength(); ++i) {
        const auto& pos = positions.Get(i);
        if (board.isEmpty(pos.x, pos.y)) {
            int score = scoreMove(board, pos.x, pos.y, player);
            candidates.AppendInPlace(Move(pos.x, pos.y, score));
            scored++;
            
            if (scored >= scoreLimit && candidates.GetLength() >= Config::TOP_K_CANDIDATES) {
                candidates.SortInPlace();
                if (candidates[Config::TOP_K_CANDIDATES - 1].score > 100) {
                    break;
                }
            }
        }
    }
    
    sortAndPrune(candidates, Config::TOP_K_CANDIDATES);
    
    return candidates;
}

} // namespace tictactoe

