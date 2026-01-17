#include "board/sparse_board.h"
#include "engine/search_engine.h"
#include "engine/config.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace tictactoe;

std::string escapeJSON(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '"') result += "\\\"";
        else if (c == '\\') result += "\\\\";
        else if (c == '\n') result += "\\n";
        else if (c == '\r') result += "\\r";
        else if (c == '\t') result += "\\t";
        else result += c;
    }
    return result;
}

int extractInt(const std::string& input, const std::string& key) {
    size_t pos = input.find("\"" + key + "\"");
    if (pos == std::string::npos) return 0;
    
    size_t colonPos = input.find(":", pos);
    if (colonPos == std::string::npos) return 0;
    
    size_t numStart = colonPos + 1;
    while (numStart < input.length() && (input[numStart] == ' ' || input[numStart] == '\t')) {
        numStart++;
    }
    
    size_t numEnd = numStart;
    while (numEnd < input.length() && 
           (isdigit(input[numEnd]) || input[numEnd] == '-' || input[numEnd] == '+')) {
        numEnd++;
    }
    
    if (numEnd > numStart) {
        return std::stoi(input.substr(numStart, numEnd - numStart));
    }
    return 0;
}

std::string extractString(const std::string& input, const std::string& key) {
    size_t pos = input.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    
    size_t colonPos = input.find(":", pos);
    if (colonPos == std::string::npos) return "";
    
    size_t quoteStart = input.find("\"", colonPos);
    if (quoteStart == std::string::npos) return "";
    
    size_t quoteEnd = input.find("\"", quoteStart + 1);
    if (quoteEnd == std::string::npos) return "";
    
    return input.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
}

Player parsePlayer(const std::string& str) {
    if (str == "X" || str == "x") return Player::X;
    if (str == "O" || str == "o") return Player::O;
    return Player::None;
}

std::string playerToString(Player player) {
    if (player == Player::X) return "X";
    if (player == Player::O) return "O";
    return "None";
}

std::vector<std::tuple<int, int, Player>> parseMovesWithPlayers(const std::string& input) {
    std::vector<std::tuple<int, int, Player>> moves;
    
    size_t movesPos = input.find("\"moves\"");
    if (movesPos == std::string::npos) return moves;
    
    size_t arrayStart = input.find("[", movesPos);
    if (arrayStart == std::string::npos) return moves;
    
    size_t pos = arrayStart + 1;
    while (pos < input.length()) {
        while (pos < input.length() && (input[pos] == ' ' || input[pos] == '\t' || input[pos] == '\n' || input[pos] == '\r')) {
            pos++;
        }
        
        if (pos >= input.length() || input[pos] == ']') {
            break;
        }
        
        if (input[pos] != '{') {
            pos++;
            continue;
        }
        
        size_t objStart = pos;
        size_t objEnd = pos + 1;
        int braceCount = 1;
        
        while (objEnd < input.length() && braceCount > 0) {
            if (input[objEnd] == '{') braceCount++;
            else if (input[objEnd] == '}') braceCount--;
            objEnd++;
        }
        
        if (braceCount != 0) {
            pos = objEnd;
            continue;
        }
        
        std::string objStr = input.substr(objStart, objEnd - objStart);
        size_t xPos = objStr.find("\"x\"");
        if (xPos == std::string::npos) {
            pos = objEnd;
            continue;
        }
        
        size_t xColon = objStr.find(":", xPos);
        if (xColon == std::string::npos) {
            pos = objEnd;
            continue;
        }
        
        size_t xValueStart = xColon + 1;
        while (xValueStart < objStr.length() && (objStr[xValueStart] == ' ' || objStr[xValueStart] == '\t')) {
            xValueStart++;
        }
        
        size_t xValueEnd = xValueStart;
        while (xValueEnd < objStr.length() && 
               (isdigit(objStr[xValueEnd]) || objStr[xValueEnd] == '-' || objStr[xValueEnd] == '+')) {
            xValueEnd++;
        }
        
        if (xValueEnd == xValueStart) {
            pos = objEnd;
            continue;
        }
        
        int x = std::stoi(objStr.substr(xValueStart, xValueEnd - xValueStart));
        
        // Parse y
        size_t yPos = objStr.find("\"y\"");
        if (yPos == std::string::npos) {
            pos = objEnd;
            continue;
        }
        
        size_t yColon = objStr.find(":", yPos);
        if (yColon == std::string::npos) {
            pos = objEnd;
            continue;
        }
        
        size_t yValueStart = yColon + 1;
        while (yValueStart < objStr.length() && (objStr[yValueStart] == ' ' || objStr[yValueStart] == '\t')) {
            yValueStart++;
        }
        
        size_t yValueEnd = yValueStart;
        while (yValueEnd < objStr.length() && 
               (isdigit(objStr[yValueEnd]) || objStr[yValueEnd] == '-' || objStr[yValueEnd] == '+')) {
            yValueEnd++;
        }
        
        if (yValueEnd == yValueStart) {
            pos = objEnd;
            continue;
        }
        
        int y = std::stoi(objStr.substr(yValueStart, yValueEnd - yValueStart));
        
        Player player = Player::X;
        size_t playerPos = objStr.find("\"player\"");
        if (playerPos != std::string::npos) {
            size_t playerColon = objStr.find(":", playerPos);
            if (playerColon != std::string::npos) {
                size_t playerQuoteStart = objStr.find("\"", playerColon);
                if (playerQuoteStart != std::string::npos) {
                    size_t playerQuoteEnd = objStr.find("\"", playerQuoteStart + 1);
                    if (playerQuoteEnd != std::string::npos) {
                        std::string playerStr = objStr.substr(playerQuoteStart + 1, 
                                                           playerQuoteEnd - playerQuoteStart - 1);
                        player = parsePlayer(playerStr);
                    }
                }
            }
        }
        
        moves.push_back({x, y, player});
        pos = objEnd;
    }
    
    return moves;
}

void serializeBoard(const SparseBoard& board, std::ostream& out) {
    BoundingBox bbox = board.getBoundingBox();
    auto occupied = board.getOccupiedPositions();
    
    out << "    \"cells\": [";
    bool first = true;
    for (int i = 0; i < occupied.GetLength(); ++i) {
        const auto& pos = occupied.Get(i);
        Player player = board.at(pos.x, pos.y);
        if (player != Player::None) {
            if (!first) out << ", ";
            out << "{\"x\": " << pos.x << ", \"y\": " << pos.y 
                << ", \"player\": \"" << playerToString(player) << "\"}";
            first = false;
        }
    }
    out << "],\n";
    
    out << "    \"bbox\": {\"min_x\": " << bbox.getMinX() 
        << ", \"max_x\": " << bbox.getMaxX() 
        << ", \"min_y\": " << bbox.getMinY() 
        << ", \"max_y\": " << bbox.getMaxY() << "}\n";
}

void serializeStats(const SearchStats& stats, std::ostream& out) {
    std::string decisionType;
    switch (stats.getDecisionType()) {
        case DecisionType::IMMEDIATE_WIN:
            decisionType = "IMMEDIATE_WIN";
            break;
        case DecisionType::IMMEDIATE_BLOCK:
            decisionType = "IMMEDIATE_BLOCK";
            break;
        case DecisionType::DANGEROUS_THREAT:
            decisionType = "DANGEROUS_THREAT";
            break;
        case DecisionType::THREAT_SOLVER:
            decisionType = "THREAT_SOLVER";
            break;
        case DecisionType::NEGAMAX_SEARCH:
            decisionType = "NEGAMAX_SEARCH";
            break;
    }
    
    out << "    \"time_ms\": " << stats.getTimeMs() << ",\n";
    out << "    \"decision_type\": \"" << decisionType << "\",\n";
    out << "    \"depth_reached\": " << stats.getDepthReached() << ",\n";
    out << "    \"nodes_searched\": " << stats.getNodesSearched() << ",\n";
    out << "    \"final_score\": " << stats.getFinalScore() << ",\n";
    
    out << "    \"principal_variation\": [";
    bool first = true;
    for (int i = 0; i < stats.getPvLength(); ++i) {
        Move pvMove = stats.getPrincipalVariation(i);
        if (pvMove.x != 0 || pvMove.y != 0) {
            if (!first) out << ", ";
            out << "{\"x\": " << pvMove.x << ", \"y\": " << pvMove.y << "}";
            first = false;
        }
    }
    out << "]\n";
}

void outputError(const std::string& error) {
    std::cout << "{\n";
    std::cout << "  \"success\": false,\n";
    std::cout << "  \"error\": \"" << escapeJSON(error) << "\"\n";
    std::cout << "}\n";
}

void outputSuccess(const SparseBoard& board, const Move* move = nullptr, 
                   const SearchStats* stats = nullptr, bool gameOver = false, 
                   Player winner = Player::None, Player movePlayer = Player::None) {
    std::cout << "{\n";
    std::cout << "  \"success\": true,\n";
    std::cout << "  \"board\": {\n";
    serializeBoard(board, std::cout);
    std::cout << "  },\n";
    
    if (move != nullptr) {
        std::string playerStr = movePlayer != Player::None ? playerToString(movePlayer) : "X";
        std::cout << "  \"move\": {\"x\": " << move->x << ", \"y\": " << move->y 
                  << ", \"player\": \"" << playerStr << "\"},\n";
    }
    
    if (stats != nullptr) {
        std::cout << "  \"stats\": {\n";
        serializeStats(*stats, std::cout);
        std::cout << "  },\n";
    }
    
    std::cout << "  \"game_over\": " << (gameOver ? "true" : "false") << ",\n";
    
    if (gameOver && winner != Player::None) {
        std::cout << "  \"winner\": \"" << playerToString(winner) << "\",\n";
    } else {
        std::cout << "  \"winner\": null,\n";
    }
    
    std::cout << "  \"is_terminal\": " << (board.isTerminal() ? "true" : "false") << "\n";
    std::cout << "}\n";
}

int main(int argc, char* argv[]) {
    try {
        std::string input;
        std::string line;
        while (std::getline(std::cin, line)) {
            input += line;
        }
        
        if (input.empty()) {
            outputError("Empty input");
            return 1;
        }
        
        std::string command = extractString(input, "command");
        if (command.empty()) {
            outputError("Missing 'command' field");
            return 1;
        }
        
        int winLength = extractInt(input, "win_length");
        if (winLength < 3) winLength = Config::WIN_LENGTH;
        if (winLength > 20) winLength = 20;
        
        auto movesWithPlayers = parseMovesWithPlayers(input);
        
        SparseBoard board(winLength);
        for (const auto& [x, y, player] : movesWithPlayers) {
            if (!board.makeMove(x, y, player)) {
                outputError("Invalid move in history: (" + std::to_string(x) + ", " + std::to_string(y) + 
                           "), player: " + playerToString(player) + ", total moves: " + std::to_string(movesWithPlayers.size()));
                return 1;
            }
        }
        
        std::string currentPlayerStr = extractString(input, "current_player");
        Player currentPlayer = parsePlayer(currentPlayerStr);
        if (currentPlayer == Player::None && !currentPlayerStr.empty()) {
            outputError("Invalid current_player: " + currentPlayerStr);
            return 1;
        }
        
        int timeMs = extractInt(input, "time_ms");
        if (timeMs <= 0) timeMs = Config::DEFAULT_TIME_MS;
        
        if (command == "make_move") {
            int moveX = 0, moveY = 0;
            
            size_t movesPos = input.find("\"moves\"");
            size_t movesArrayEnd = input.find("]", movesPos);
            if (movesArrayEnd != std::string::npos) {
                size_t xPos = input.find("\"x\"", movesArrayEnd);
                if (xPos != std::string::npos) {
                    size_t xColon = input.find(":", xPos);
                    if (xColon != std::string::npos) {
                        size_t xValueStart = xColon + 1;
                        while (xValueStart < input.length() && (input[xValueStart] == ' ' || input[xValueStart] == '\t')) {
                            xValueStart++;
                        }
                        size_t xValueEnd = xValueStart;
                        while (xValueEnd < input.length() && 
                               (isdigit(input[xValueEnd]) || input[xValueEnd] == '-' || input[xValueEnd] == '+')) {
                            xValueEnd++;
                        }
                        if (xValueEnd > xValueStart) {
                            moveX = std::stoi(input.substr(xValueStart, xValueEnd - xValueStart));
                        }
                    }
                }
                
                size_t yPos = input.find("\"y\"", movesArrayEnd);
                if (yPos != std::string::npos) {
                    size_t yColon = input.find(":", yPos);
                    if (yColon != std::string::npos) {
                        size_t yValueStart = yColon + 1;
                        while (yValueStart < input.length() && (input[yValueStart] == ' ' || input[yValueStart] == '\t')) {
                            yValueStart++;
                        }
                        size_t yValueEnd = yValueStart;
                        while (yValueEnd < input.length() && 
                               (isdigit(input[yValueEnd]) || input[yValueEnd] == '-' || input[yValueEnd] == '+')) {
                            yValueEnd++;
                        }
                        if (yValueEnd > yValueStart) {
                            moveY = std::stoi(input.substr(yValueStart, yValueEnd - yValueStart));
                        }
                    }
                }
            } else {
                moveX = extractInt(input, "x");
                moveY = extractInt(input, "y");
            }
            
            if (!board.makeMove(moveX, moveY, currentPlayer)) {
                outputError("Invalid move: (" + std::to_string(moveX) + ", " + std::to_string(moveY) + ")");
                return 1;
            }
            
            bool gameOver = board.isTerminal();
            Player winner = Player::None;
            if (gameOver) {
                auto history = board.getMoveHistory();
                if (!history.Empty()) {
                    const auto& lastMove = history.Back();
                    if (board.isWin(lastMove.x, lastMove.y, lastMove.player)) {
                        winner = lastMove.player;
                    }
                }
            }
            
            Move madeMove(moveX, moveY);
            outputSuccess(board, &madeMove, nullptr, gameOver, winner, currentPlayer);
            
        } else if (command == "ai_move") {
            SearchEngine engine(winLength);
            
            Move aiMove = engine.findBestMove(board, currentPlayer, timeMs);
            SearchStats stats = engine.getStats();
            
            if (!board.makeMove(aiMove.x, aiMove.y, currentPlayer)) {
                outputError("AI generated invalid move: (" + std::to_string(aiMove.x) + ", " + std::to_string(aiMove.y) + ")");
                return 1;
            }
            
            bool gameOver = board.isTerminal();
            Player winner = Player::None;
            if (gameOver) {
                auto history = board.getMoveHistory();
                if (!history.Empty()) {
                    const auto& lastMove = history.Back();
                    if (board.isWin(lastMove.x, lastMove.y, lastMove.player)) {
                        winner = lastMove.player;
                    }
                }
            }
            
            outputSuccess(board, &aiMove, &stats, gameOver, winner, currentPlayer);
            
        } else if (command == "get_state") {
            outputSuccess(board, nullptr, nullptr, board.isTerminal(), Player::None);
            
        } else {
            outputError("Unknown command: " + command);
            return 1;
        }
        
        return 0;
        
    } catch (const std::exception& e) {
        outputError(std::string("Exception: ") + e.what());
        return 1;
    } catch (...) {
        outputError("Unknown exception occurred");
        return 1;
    }
}

