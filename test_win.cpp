#include "board/sparse_board.h"
#include <iostream>

using namespace tictactoe;

int main() {
    std::cout << "Testing win detection...\n\n";
    
    SparseBoard board(5);
    
    std::cout << "Creating 5 in a row horizontally...\n";
    for (int i = 0; i < 5; ++i) {
        board.makeMove(i, 0, Player::X);
        std::cout << "Move " << i << ": (" << i << ", 0)\n";
        
        if (board.isWin(i, 0, Player::X)) {
            std::cout << "  ✓ Win detected after move " << i << "!\n";
        } else {
            std::cout << "  ✗ No win detected after move " << i << "\n";
        }
    }
    
    std::cout << "\nFinal check:\n";
    if (board.isTerminal()) {
        std::cout << "  ✓ Board is terminal (game over)\n";
    } else {
        std::cout << "  ✗ Board is not terminal\n";
    }
    
    std::cout << "\n\nTesting vertical win...\n";
    SparseBoard board2(5);
    for (int i = 0; i < 5; ++i) {
        board2.makeMove(0, i, Player::O);
        if (board2.isWin(0, i, Player::O)) {
            std::cout << "  ✓ Win detected at (0, " << i << ")\n";
            break;
        }
    }
    
    std::cout << "\n\nTesting diagonal win...\n";
    SparseBoard board3(5);
    for (int i = 0; i < 5; ++i) {
        board3.makeMove(i, i, Player::X);
        if (board3.isWin(i, i, Player::X)) {
            std::cout << "  ✓ Win detected at (" << i << ", " << i << ")\n";
            break;
        }
    }
    
    return 0;
}

