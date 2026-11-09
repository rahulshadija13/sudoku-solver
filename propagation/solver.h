#ifndef SOLVER_H
#define SOLVER_H

#include <vector>

struct SolveResult {
    bool solved;                            // true if a complete solution was found
    std::vector<std::vector<int>> board;    // the solved grid (the original puzzle if unsolved)
    int guesses;                            // number of digits tried during the search
    double timeMs;                          // solve time in milliseconds
};

// puzzle is a 9x9 grid, 0 marks an empty cell
SolveResult solveSudoku(const std::vector<std::vector<int>>& puzzle);

#endif