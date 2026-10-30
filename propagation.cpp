/*
 * Sudoku solver: constraint propagation + backtracking search
 *
 * How it works
 *   1. Every cell keeps a set of candidate digits: cand[cell][digit] is true
 *      while that digit is still possible in that cell.
 *   2. propagate() fills every cell the rules force, until nothing changes:
 *        - naked single:  an empty cell with exactly one candidate left
 *        - hidden single: a digit that fits in only one cell of a row,
 *                         column or box
 *      If a cell or a digit has no place left, the board is a dead end.
 *   3. If the puzzle is still unsolved, assign() guesses a digit for the empty
 *      cell with the fewest candidates. The guess is made on a copy of the
 *      board, so a wrong guess is simply thrown away.
 *
 * Board layout
 *   board[row][col] holds the digit, 0 means empty.
 *   A cell number is row*9 + col, so cell 0 is the top-left corner and
 *   cell 80 is the bottom-right corner.
 *
 * Build and run
 *   g++ -std=c++17 -O2 -o prop propagation.cpp
 *   ./prop
 *
 * Output: the solved grid, the number of guesses made, and the solve time.
 * The puzzle is hard-coded in main(); 0 marks an empty cell.
 */

#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

int guesses = 0;    // number of digits tried during the search (0 = solved by logic alone)

// forward declarations (the functions call each other)
void eliminate(vector<vector<int>>& board, vector<vector<bool>>& cand, int row, int col, int tgt);
bool checkUnit(vector<vector<int>>& board, vector<vector<bool>>& cand, vector<int>& unit, bool& changed);
bool hiddenSingle(vector<vector<int>>& board, vector<vector<bool>>& cand, bool& changed);
bool propagate(vector<vector<int>>& board, vector<vector<bool>>& cand);
vector<int> findBest(vector<vector<int>>& board, vector<vector<bool>>& cand);
bool assign(vector<vector<int>>& board, vector<vector<bool>>& cand, int row, int col);
bool solve(vector<vector<int>>& board, vector<vector<bool>>& cand);

// Removes the digits already given in the puzzle from the candidates of their peers
void traversal(vector<vector<int>>& board, vector<vector<bool>>& cand){
    for (int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            if(board[i][j] != 0){
                int tgt = board[i][j];
                eliminate(board, cand, i, j, tgt);
            }
        }
    }
}

// Removes digit tgt from the candidates of every cell in the same row, column and box
void eliminate(vector<vector<int>>& board, vector<vector<bool>>& cand, int row, int col, int tgt){
    for(int i=0; i<9; i++){
        if(cand[row*9 + i][tgt]){
            cand[row*9 + i][tgt] = false;
        }
    }

    for(int j=0; j<9; j++){
        if(cand[col + 9*j][tgt]){
            cand[col + 9*j][tgt] = false;
        }
    }

    // 3x3 box
    int boxRow = (row / 3) * 3;
    int boxCol = (col / 3) * 3;
    for(int i=0; i<3; i++){
        for(int j=0; j<3; j++){
            int cell = (boxRow + i) * 9 + (boxCol + j);
            if(cand[cell][tgt]){
                cand[cell][tgt] = false;
            }
        }
    }
}

// Hidden single check for one unit (a row, column or box). unit holds its 9 cell numbers.
// If a digit has only one empty cell left in the unit, it is placed there.
// Returns false if some digit has no place left in the unit (dead end).
// Sets changed to true if it filled a cell.
bool checkUnit(vector<vector<int>>& board, vector<vector<bool>>& cand, vector<int>& unit, bool& changed){
    for(int d=1; d<=9; d++){
        int count = 0;
        int where = -1;
        bool placed = false;

        for(int k=0; k<9; k++){
            int cell = unit[k];
            if(board[cell/9][cell%9] == d){
                placed = true;
            }
            else if(board[cell/9][cell%9] == 0 && cand[cell][d]){
                count++;
                where = cell;
            }
        }

        if(placed){
            continue;
        }
        if(count == 0){
            return false;      // this digit can't go anywhere in the unit
        }
        else if(count == 1){
            board[where/9][where%9] = d;
            eliminate(board, cand, where/9, where%9, d);
            changed = true;
        }
    }
    return true;
}

// Builds all 27 units (9 rows, 9 columns, 9 boxes) and runs checkUnit on each
bool hiddenSingle(vector<vector<int>>& board, vector<vector<bool>>& cand, bool& changed){
    for(int i=0; i<9; i++){
        vector<int> row, col, box;
        for(int j=0; j<9; j++){
            row.push_back(i*9 + j);
            col.push_back(j*9 + i);
            box.push_back(((i/3)*3 + j/3)*9 + (i%3)*3 + j%3);
        }
        if(!checkUnit(board, cand, row, changed)) return false;
        if(!checkUnit(board, cand, col, changed)) return false;
        if(!checkUnit(board, cand, box, changed)) return false;
    }
    return true;
}

// Fills every forced cell (naked and hidden singles), repeating until nothing changes.
// Returns false if a contradiction shows up (a cell or a digit with no place left).
bool propagate(vector<vector<int>>& board, vector<vector<bool>>& cand){
    bool changed = true;

    while(changed){
        changed = false;

        // naked singles
        for(int i=0; i<9; i++){
            for(int j=0; j<9; j++){
                if(board[i][j] == 0){
                    int count = 0;
                    int sol = 0;

                    for(int k=1; k<=9; k++){
                        if(cand[9*i + j][k]){
                            count++;
                            sol = k;
                        }
                    }
                    if(count == 0){
                        return false;
                    }
                    else if(count == 1){
                        board[i][j] = sol;
                        eliminate(board, cand, i, j, sol);
                        changed = true;
                    }
                }
            }
        }

        // hidden singles
        if(!hiddenSingle(board, cand, changed)){
            return false;
        }
    }
    return true;
}

// Finds the empty cell with the fewest candidates.
// Returns {row, col, candidate count}, or an empty vector if no empty cell is left.
vector<int> findBest(vector<vector<int>>& board, vector<vector<bool>>& cand){
    vector<int> bestCell;
    int bestcnt = 10;

    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            if(board[i][j] == 0){
                int count = 0;

                for(int k=1; k<=9; k++){
                    if(cand[9*i + j][k]){
                        count++;
                    }
                }
                if(count < bestcnt){
                    bestcnt = count;
                    bestCell = {i, j, bestcnt};
                }
            }
        }
    }
    return bestCell;
}

// Guesses each candidate of cell (row, col) in turn, on a copy of the board.
// A failed guess is discarded; a successful one is copied back into board.
bool assign(vector<vector<int>>& board, vector<vector<bool>>& cand, int row, int col){
    for(int d=1; d<=9; d++){
        if(cand[row*9 + col][d]){
            guesses++;
            vector<vector<int>> next = board;
            vector<vector<bool>> candid = cand;
            next[row][col] = d;
            eliminate(next, candid, row, col, d);
            if(solve(next, candid)){
                board = next;      // success: pass the solution back up
                cand = candid;
                return true;
            }
        }
    }
    return false;                  // every candidate failed
}

// Propagate first, then guess only if the puzzle is still unsolved.
// Returns true if board is now completely solved.
bool solve(vector<vector<int>>& board, vector<vector<bool>>& cand){
    if(!propagate(board, cand)){
        return false;
    }
    vector<int> best = findBest(board, cand);
    if(best.empty()){
        return true;               // no empty cells left -> solved
    }
    return assign(board, cand, best[0], best[1]);
}

int main(){
    // 0 = empty cell
        vector<vector<int>> board = {
        {0,0,0,0,6,1,0,0,0},
        {3,0,0,4,0,0,8,0,0},
        {0,6,0,0,0,3,7,0,0},
        {5,0,0,7,0,0,0,9,0},
        {0,4,0,1,9,0,0,2,0},
        {6,0,0,0,0,0,0,1,0},
        {4,0,0,9,0,0,5,0,0},
        {0,0,2,3,0,0,0,7,0},
        {0,0,0,0,0,0,0,0,0}
    };
    // cand[cell][digit]: all 9 digits are possible everywhere at the start (index 0 unused)
    vector<vector<bool>> cand(81, vector<bool>(10, true));

    auto start = chrono::steady_clock::now();

    traversal(board, cand);
    bool ok = solve(board, cand);

    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> ms = end - start;

    if(ok){
        for(int i=0; i<9; i++){
            for(int j=0; j<9; j++) cout << board[i][j] << " ";
            cout << "\n";
        }
    } else {
        cout << "No solution\n";
    }
    cout << "guesses: " << guesses << "\n";
    cout << "time: " << ms.count() << " ms\n";
    return 0;
}
