 /*
 * Sudoku solver: plain backtracking
 *
 * How it works
 *   Cells are visited left to right, top to bottom. For each empty cell the
 *   solver tries the digits 1 to 9. A digit is accepted only if it does not
 *   already appear in the same row, column or 3x3 box (isSafe). If the rest of
 *   the board cannot be completed with that digit, it is removed again and the
 *   next digit is tried.
 *
 * Board layout
 *   board[row][col] holds the digit, 0 means empty.
 *
 * Build and run
 *   g++ -std=c++17 -O2 -o bt backtracking.cpp
 *   ./bt
 *
 * Output: the puzzle, the solved grid, the number of guesses made, and the
 * solve time. The puzzle is hard-coded in main(); 0 marks an empty cell.
 */

#include <iostream>
#include <vector>
#include <chrono>
using namespace std;

int guesses = 0;    // number of digits tried in a cell (used to compare with the propagation solver)

// Returns true if digit dig can be placed at (row, col) without a clash
// in its row, column or 3x3 box
bool isSafe(vector<vector<int>>& board, int row, int col, int dig){
    for(int j=0; j<9; j++){
        if(board[row][j] == dig){
            return false;
        }
    }

    for(int i=0; i<9; i++){
        if(board[i][col] == dig){
            return false;
        }
    }

    int srow = (row/3)*3;
    int scol = (col/3)*3;
    
    for(int i=srow; i<=srow+2; i++){
        for(int j=scol; j<=scol+2; j++){
            if(board[i][j] == dig){
                return false;
            }
        }
    }

    return true;
}

// Fills the board from cell (row, col) onwards.
// Returns true once every cell is filled, false if this path is a dead end.
bool sudokuSolver(vector<vector<int>>& board, int row, int col){
    if(row == 9) return true;

    int nextrow = row, nextcol = col +1;
    if(nextcol == 9){
        nextrow += 1;
        nextcol = 0;
    }

    // cell already filled: move on
    if(board[row][col] != 0){
        return sudokuSolver(board, nextrow, nextcol);
    }

    for(int dig=1; dig<10; dig++){
        if(isSafe(board, row, col, dig)){
            guesses++;
            board[row][col] = dig;
            if(sudokuSolver(board, nextrow, nextcol)) return true;

            board[row][col] = 0;   // backtrack: undo the digit and try the next one
        }
    }
    
    return false;
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

    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            cout<<board[i][j]<<" ";
        }
        cout<<endl;
    }

    cout << "Starting solver..." << endl;

    auto start = chrono::steady_clock::now();

    bool ok = sudokuSolver(board,0,0);

    auto end = chrono::steady_clock::now();
    chrono::duration<double, milli> ms = end - start;
    
    if(ok){
        for(int i=0; i<9; i++){
            for(int j=0; j<9; j++){
                cout<<board[i][j]<<" ";
            }
            cout<<endl;
        }
    } else {
        cout << "No solution" << endl;
    }
    cout << "guesses: " << guesses << endl;
    cout << "time: " << ms.count() << " ms" << endl;
    return 0;

}
