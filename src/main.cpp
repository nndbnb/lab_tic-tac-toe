#include "board/sparse_board.h"
#include "engine/search_engine.h"
#include "engine/config.h"
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>

using namespace tictactoe;

void printBoard(const SparseBoard& board) {
    BoundingBox bbox = board.getBoundingBox();
    
    int margin = 2;
    int min_x = bbox.getMinX() - margin;
    int max_x = bbox.getMaxX() + margin;
    int min_y = bbox.getMinY() - margin;
    int max_y = bbox.getMaxY() + margin;
    
    std::cout << "   ";
    for (int x = min_x; x <= max_x; ++x) {
        std::cout << std::setw(3) << x;
    }
    std::cout << "\n";
    
    for (int y = max_y; y >= min_y; --y) {
        std::cout << std::setw(3) << y << " ";
        for (int x = min_x; x <= max_x; ++x) {
            Player cell = board.at(x, y);
            if (cell == Player::X) {
                std::cout << " X ";
            } else if (cell == Player::O) {
                std::cout << " O ";
            } else {
                std::cout << " . ";
            }
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

bool parseMove(const std::string& input, int& x, int& y) {
    std::istringstream iss(input);
    return !!(iss >> x >> y);
}

std::string formatTime(int timeMs) {
    if (timeMs < 1000) {
        return std::to_string(timeMs) + " ms";
    } else {
        double seconds = timeMs / 1000.0;
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << seconds << " s";
        return oss.str();
    }
}

void printBriefReport(const SearchStats& stats) {
    std::string decisionType;
    switch (stats.getDecisionType()) {
        case DecisionType::IMMEDIATE_WIN:
            decisionType = "Immediate win";
            break;
        case DecisionType::IMMEDIATE_BLOCK:
            decisionType = "Immediate block";
            break;
        case DecisionType::DANGEROUS_THREAT:
            decisionType = "Dangerous threat block";
            break;
        case DecisionType::THREAT_SOLVER:
            decisionType = "Threat-based forced win";
            break;
        case DecisionType::NEGAMAX_SEARCH:
            decisionType = "Negamax search";
            break;
    }
    
    std::cout << "Time: " << formatTime(stats.getTimeMs()) << " | Method: " << decisionType;
    if (stats.getDecisionType() == DecisionType::NEGAMAX_SEARCH) {
        std::cout << " (depth " << stats.getDepthReached() << ")";
    }
    std::cout << "\n";
}

void printDetailedStats(const SearchStats& stats) {
    std::cout << "\n=== Detailed Search Statistics ===\n";
    std::cout << "Decision method: ";
    switch (stats.getDecisionType()) {
        case DecisionType::IMMEDIATE_WIN:
            std::cout << "Immediate Win (found winning move)\n";
            break;
        case DecisionType::IMMEDIATE_BLOCK:
            std::cout << "Immediate Block (blocked opponent's winning move)\n";
            break;
        case DecisionType::DANGEROUS_THREAT:
            std::cout << "Dangerous Threat Block (blocked opponent's Open-(N-2) threat)\n";
            break;
        case DecisionType::THREAT_SOLVER:
            std::cout << "Threat Solver (found forced win through threats)\n";
            break;
        case DecisionType::NEGAMAX_SEARCH:
            std::cout << "Negamax Search (full alpha-beta search)\n";
            break;
    }
    
    std::cout << "Time: " << formatTime(stats.getTimeMs()) << "\n";
    std::cout << "Nodes searched: " << stats.getNodesSearched() << "\n";
    
    if (stats.getDecisionType() == DecisionType::NEGAMAX_SEARCH) {
        std::cout << "Depth reached: " << stats.getDepthReached() << "\n";
        std::cout << "Final score: " << stats.getFinalScore() << "\n";
        
        if (stats.getPvLength() > 0) {
            std::cout << "Principal variation: ";
            for (int i = 0; i < stats.getPvLength() && i < 10; ++i) {
                Move pvMove = stats.getPrincipalVariation(i);
                std::cout << "(" << pvMove.x 
                         << "," << pvMove.y << ") ";
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "===================================\n\n";
}

int main(int argc, char* argv[]) {
    std::cout << "=== Infinite Tic-Tac-Toe Engine ===\n\n";
    
    int winLength = Config::WIN_LENGTH;
    if (argc > 1) {
        try {
            winLength = std::stoi(argv[1]);
            if (winLength < 3) {
                std::cerr << "Win length must be at least 3. Using minimum: 3\n";
                winLength = 3;
            } else if (winLength > 20) {
                std::cerr << "Win length too large (max 20). Using maximum: 20\n";
                winLength = 20;
            }
        } catch (...) {
            std::cerr << "Invalid argument. Using default win length: " << Config::WIN_LENGTH << "\n";
            winLength = Config::WIN_LENGTH;
        }
    }
    
    std::cout << "Win condition: " << winLength << " in a row\n";
    std::cout << "Commands: 'x y' to make move, 'quit' to exit\n\n";
    
    std::cout << "Choose your player (X or O, default X): ";
    std::string playerChoice;
    std::getline(std::cin, playerChoice);
    
    Player humanPlayer = Player::X;
    Player aiPlayer = Player::O;
    
    if (playerChoice == "O" || playerChoice == "o") {
        humanPlayer = Player::O;
        aiPlayer = Player::X;
        std::cout << "You are playing as O, AI is X\n\n";
    } else {
        std::cout << "You are playing as X, AI is O\n\n";
    }
    
    std::cout << "Who goes first? (you/ai, default you): ";
    std::string firstMove;
    std::getline(std::cin, firstMove);
    
    bool aiFirst = (firstMove == "ai" || firstMove == "AI");
    
    std::cout << "Enable detailed decision logs? (yes/no, default no): ";
    std::string detailedLogs;
    std::getline(std::cin, detailedLogs);
    
    bool enableDetailedLogs = (detailedLogs == "yes" || detailedLogs == "y" || detailedLogs == "Y");
    
    SparseBoard board(winLength);
    SearchEngine engine(winLength);
    Player currentPlayer = Player::X;
    
    if (aiFirst) {
        std::cout << "AI makes the first move...\n";
        if (aiPlayer == Player::X) {
            board.makeMove(0, 0, Player::X);
            std::cout << "AI plays: (0, 0)\n\n";
            currentPlayer = Player::O;
        } else {
            currentPlayer = Player::X;
        }
    }
    
    while (true) {
        printBoard(board);
        
        auto history = board.getMoveHistory();
        if (!history.Empty()) {
            if (board.isTerminal()) {
                const auto& lastMove = history.Back();
                Player winner = lastMove.player;
                
                if (board.isWin(lastMove.x, lastMove.y, winner)) {
                    std::cout << "Player " 
                             << (winner == Player::X ? "X" : "O") 
                             << " wins!\n";
                    break;
                } else {
                    Player opponent = (winner == Player::X) ? Player::O : Player::X;
                    auto occupied = board.getOccupiedPositions();
                    for (int i = 0; i < occupied.GetLength(); ++i) {
                        const auto& pos = occupied.Get(i);
                        if (board.at(pos.x, pos.y) == opponent && 
                            board.isWin(pos.x, pos.y, opponent)) {
                            std::cout << "Player " 
                                     << (opponent == Player::X ? "X" : "O") 
                                     << " wins!\n";
                            break;
                        }
                    }
                    break;
                }
            }
        }
        
        bool isHumanTurn = (currentPlayer == humanPlayer);
        
        if (isHumanTurn) {
            std::cout << "Player " << (currentPlayer == Player::X ? "X" : "O") << " (You) to move.\n";
            std::cout << "Enter coordinates (x y): ";
            
            std::string input;
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "q" || input == "exit") {
                break;
            }
            
            int x, y;
            if (!parseMove(input, x, y)) {
                std::cout << "Invalid input. Please enter two numbers: x y\n";
                continue;
            }
            
            if (!board.makeMove(x, y, currentPlayer)) {
                std::cout << "Invalid move. Cell is already occupied or invalid.\n";
                continue;
            }
        } else {
            std::cout << "Player " << (currentPlayer == Player::X ? "X" : "O") << " (AI) is thinking...\n";
            Move aiMove = engine.findBestMove(board, currentPlayer, Config::DEFAULT_TIME_MS);
            
            auto stats = engine.getStats();
            
            if (board.makeMove(aiMove.x, aiMove.y, currentPlayer)) {
                std::cout << "AI plays: (" << aiMove.x << ", " << aiMove.y << ")\n";
                printBriefReport(stats);
                
                if (enableDetailedLogs) {
                    printDetailedStats(stats);
                } else {
                    std::cout << "\n";
                }
            } else {
                std::cout << "AI error: Invalid move generated!\n";
                std::cout << "  Attempted move: (" << aiMove.x << ", " << aiMove.y << ")\n";
                std::cout << "  Cell is " << (board.isEmpty(aiMove.x, aiMove.y) ? "empty" : "occupied") << "\n";
                std::cout << "  Decision type: ";
                switch (stats.getDecisionType()) {
                    case DecisionType::IMMEDIATE_WIN:
                        std::cout << "Immediate Win\n";
                        break;
                    case DecisionType::IMMEDIATE_BLOCK:
                        std::cout << "Immediate Block\n";
                        break;
                    case DecisionType::DANGEROUS_THREAT:
                        std::cout << "Dangerous Threat\n";
                        break;
                    case DecisionType::THREAT_SOLVER:
                        std::cout << "Threat Solver\n";
                        break;
                    case DecisionType::NEGAMAX_SEARCH:
                        std::cout << "Negamax Search (depth " << stats.getDepthReached() << ")\n";
                        break;
                }
                std::cout << "  This should not happen - fallback logic failed!\n";
                break;
            }
        }
        
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }
    
    std::cout << "Game over. Thanks for playing!\n";
    return 0;
}

