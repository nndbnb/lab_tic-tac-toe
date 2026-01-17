#include "board/sparse_board.h"
#include <cassert>
#include <iostream>
#include <cstdint>

using namespace tictactoe;

void testBasicOperations() {
    std::cout << "Testing basic board operations...\n";
    
    SparseBoard board(5);
    
    assert(board.isEmpty(0, 0));
    assert(board.at(0, 0) == Player::None);
    
    assert(board.makeMove(0, 0, Player::X));
    assert(!board.isEmpty(0, 0));
    assert(board.at(0, 0) == Player::X);
    
    assert(!board.makeMove(0, 0, Player::O));
    
    board.undoMove(0, 0);
    assert(board.isEmpty(0, 0));
    
    std::cout << "  ✓ Basic operations passed\n";
}

void testWinDetection() {
    std::cout << "Testing win detection...\n";
    
    SparseBoard board(5);
    
    for (int i = 0; i < 5; ++i) {
        board.makeMove(i, 0, Player::X);
    }
    
    assert(board.isWin(4, 0, Player::X));
    assert(board.isTerminal());
    
    std::cout << "  ✓ Win detection passed\n";
}

void testZobristHash() {
    std::cout << "Testing Zobrist hash...\n";
    
    SparseBoard board(5);
    uint64_t hash1 = board.getZobristHash();
    
    board.makeMove(0, 0, Player::X);
    uint64_t hash2 = board.getZobristHash();
    assert(hash1 != hash2);
    
    board.undoMove(0, 0);
    uint64_t hash3 = board.getZobristHash();
    assert(hash1 == hash3);
    
    std::cout << "  ✓ Zobrist hash passed\n";
}

void testBoundingBox() {
    std::cout << "Testing bounding box...\n";
    
    SparseBoard board(5);
    BoundingBox bbox = board.getBoundingBox();
    
    board.makeMove(10, 20, Player::X);
    bbox = board.getBoundingBox();
    assert(bbox.getMinX() == 10 && bbox.getMaxX() == 10);
    assert(bbox.getMinY() == 20 && bbox.getMaxY() == 20);
    
    board.makeMove(-5, -10, Player::O);
    bbox = board.getBoundingBox();
    assert(bbox.getMinX() == -5 && bbox.getMaxX() == 10);
    assert(bbox.getMinY() == -10 && bbox.getMaxY() == 20);
    
    std::cout << "  ✓ Bounding box passed\n";
}

void testCopyConstructor() {
    std::cout << "Testing copy constructor...\n";
    
    SparseBoard board1(5);
    board1.makeMove(1, 2, Player::X);
    board1.makeMove(3, 4, Player::O);
    
    SparseBoard board2(board1);
    assert(board2.at(1, 2) == Player::X);
    assert(board2.at(3, 4) == Player::O);
    assert(board2.getWinLength() == 5);
    
    std::cout << "  ✓ Copy constructor passed\n";
}

int main() {
    std::cout << "=== Board Tests ===\n\n";
    
    testBasicOperations();
    testWinDetection();
    testZobristHash();
    testBoundingBox();
    testCopyConstructor();
    
    std::cout << "\nAll board tests passed!\n";
    return 0;
}

