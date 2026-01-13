#include "engine/evaluator.h"
#include "board/sparse_board.h"
#include <cassert>
#include <iostream>

using namespace tictactoe;

void testPatternScoring() {
    std::cout << "Testing pattern scoring...\n";
    
    Evaluator eval5(5);
    
    int score4 = eval5.getPatternScore(4, true);
    assert(score4 > 10000);
    
    int score3 = eval5.getPatternScore(3, true);
    assert(score3 < score4);
    
    int open2 = eval5.getPatternScore(2, true);
    int closed2 = eval5.getPatternScore(2, false);
    assert(open2 > closed2);
    
    std::cout << "  ✓ Pattern scoring passed\n";
}

void testScalingForDifferentN() {
    std::cout << "Testing evaluator scaling for different N...\n";
    
    Evaluator eval3(3);
    int open2_n3 = eval3.getPatternScore(2, true);
    assert(open2_n3 > 0);
    std::cout << "  N=3: Open-2 score = " << open2_n3 << "\n";
    
    Evaluator eval5(5);
    int open4_n5 = eval5.getPatternScore(4, true);
    int open3_n5 = eval5.getPatternScore(3, true);
    assert(open4_n5 > open3_n5);
    std::cout << "  N=5: Open-4 score = " << open4_n5 << ", Open-3 score = " << open3_n5 << "\n";
    
    Evaluator eval7(7);
    int open6_n7 = eval7.getPatternScore(6, true);
    int open5_n7 = eval7.getPatternScore(5, true);
    assert(open6_n7 > open5_n7);
    std::cout << "  N=7: Open-6 score = " << open6_n7 << ", Open-5 score = " << open5_n7 << "\n";
    
    assert(open6_n7 > open4_n5);
    
    std::cout << "  ✓ Scaling for different N passed\n";
}

void testPatternDetection() {
    std::cout << "Testing pattern detection...\n";
    
    SparseBoard board(5);
    Evaluator evaluator(5);
    
    board.makeMove(0, 0, Player::X);
    board.makeMove(1, 0, Player::X);
    board.makeMove(2, 0, Player::X);
    
    auto patterns = evaluator.detectPatterns(board, 1, 0, Player::X);
    bool foundOpen3 = false;
    for (int i = 0; i < patterns.GetLength(); ++i) {
        const auto& p = patterns.Get(i);
        if (p.length == 3 && p.isOpen) {
            foundOpen3 = true;
        }
    }
    assert(foundOpen3);
    
    std::cout << "  ✓ Pattern detection passed\n";
}

void testEvaluation() {
    std::cout << "Testing position evaluation...\n";
    
    SparseBoard board(5);
    Evaluator evaluator(5);
    
    int score1 = evaluator.evaluatePosition(board, Player::X);
    
    board.makeMove(0, 0, Player::X);
    board.makeMove(1, 0, Player::X);
    int score2 = evaluator.evaluatePosition(board, Player::X);
    assert(score2 > score1);
    
    board.makeMove(0, 1, Player::O);
    board.makeMove(1, 1, Player::O);
    int score3 = evaluator.evaluatePosition(board, Player::X);
    assert(score3 < score2);
    
    std::cout << "  ✓ Evaluation passed\n";
}

void testWinMove() {
    std::cout << "Testing win move evaluation...\n";
    
    SparseBoard board(5);
    Evaluator evaluator(5);
    
    for (int i = 0; i < 4; ++i) {
        board.makeMove(i, 0, Player::X);
    }
    
    int winScore = evaluator.evaluateMove(board, 4, 0, Player::X);
    assert(winScore > 1000000);
    
    std::cout << "  ✓ Win move evaluation passed\n";
}

int main() {
    std::cout << "=== Evaluator Tests ===\n\n";
    
    testPatternScoring();
    testPatternDetection();
    testEvaluation();
    testWinMove();
    testScalingForDifferentN();
    
    std::cout << "\nAll evaluator tests passed!\n";
    return 0;
}

