#include "engine/search_engine.h"
#include "board/sparse_board.h"
#include <cassert>
#include <iostream>

using namespace tictactoe;

void testImmediateWin() {
    std::cout << "Testing immediate win detection...\n";
    
    SparseBoard board(5);
    SearchEngine engine(5);
    
    for (int i = 0; i < 4; ++i) {
        board.makeMove(i, 0, Player::X);
    }
    
    Move move = engine.findBestMove(board, Player::X, 1000);
    
    SparseBoard testBoard = board;
    assert(testBoard.makeMove(move.x, move.y, Player::X));
    assert(testBoard.isWin(move.x, move.y, Player::X));
    
    std::cout << "  ✓ Immediate win detection passed\n";
}

void testImmediateBlock() {
    std::cout << "Testing immediate block detection...\n";
    
    SparseBoard board(5);
    SearchEngine engine(5);
    
    for (int i = 0; i < 4; ++i) {
        board.makeMove(i, 0, Player::O);
    }
    
    Move move = engine.findBestMove(board, Player::X, 1000);
    
    SparseBoard testBoard = board;
    assert(testBoard.makeMove(move.x, move.y, Player::X));
    
    assert(move.x == 4 || move.x == -1);
    assert(move.y == 0);
    
    std::cout << "  ✓ Immediate block detection passed\n";
}

void testBasicSearch() {
    std::cout << "Testing basic search...\n";
    
    SparseBoard board(5);
    SearchEngine engine(5);
    
    Move move = engine.findBestMove(board, Player::X, 1000);
    
    assert(board.isEmpty(move.x, move.y));
    
    std::cout << "  ✓ Basic search passed\n";
}

void testSearchStats() {
    std::cout << "Testing search statistics...\n";
    
    SparseBoard board(5);
    SearchEngine engine(5);
    
    engine.findBestMove(board, Player::X, 1000);
    SearchStats stats = engine.getStats();
    
    assert(stats.getNodesSearched() > 0);
    assert(stats.getDepthReached() > 0);
    assert(stats.getTimeMs() >= 0);
    
    std::cout << "  ✓ Search statistics passed\n";
}

void testTranspositionTable() {
    std::cout << "Testing transposition table...\n";
    
    SparseBoard board(5);
    SearchEngine engine(5);
    
    engine.findBestMove(board, Player::X, 1000);
    
    engine.clearTT();
    Move move2 = engine.findBestMove(board, Player::X, 1000);
    
    assert(board.isEmpty(move2.x, move2.y));
    
    std::cout << "  ✓ Transposition table passed\n";
}

int main() {
    std::cout << "=== Engine Tests ===\n\n";
    
    testImmediateWin();
    testImmediateBlock();
    testBasicSearch();
    testSearchStats();
    testTranspositionTable();
    
    std::cout << "\nAll engine tests passed!\n";
    return 0;
}

