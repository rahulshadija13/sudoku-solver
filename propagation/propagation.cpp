#include <iostream>
#include <vector>
#include "solver.h"
using namespace std;

int main(){
    // 0 = empty cell
    vector<vector<int>> board = {
        {5,3,0,0,7,0,0,0,0},
        {6,0,0,1,9,5,0,0,0},
        {0,9,8,0,0,0,0,6,0},
        {8,0,0,0,6,0,0,0,3},
        {4,0,0,8,0,3,0,0,1},
        {7,0,0,0,2,0,0,0,6},
        {0,6,0,0,0,0,2,8,0},
        {0,0,0,4,1,9,0,0,5},
        {0,0,0,0,8,0,0,7,9}
    };

    SolveResult res = solveSudoku(board);

    if(res.solved){
        for(int i=0; i<9; i++){
            for(int j=0; j<9; j++) cout << res.board[i][j] << " ";
            cout << "\n";
        }
    } else {
        cout << "No solution\n";
    }
    cout << "guesses: " << res.guesses << "\n";
    cout << "time: " << res.timeMs << " ms\n";
    return 0;
}