# Sudoku Solver in C++

Two solvers for 9x9 sudoku, written to compare a plain backtracking search with constraint propagation. Both print the solved grid, the number of guesses they needed, and the solve time, so they can be compared on the same puzzle.

## The two approaches

### Backtracking (`backtracking.cpp`)

The baseline. Cells are visited left to right, top to bottom. For each empty cell the solver tries the digits 1 to 9 and accepts a digit only if it does not already appear in the same row, column or 3x3 box. If the rest of the board cannot be completed with that digit, the digit is removed and the next one is tried. There are no deductions: every decision is a guess.

### Constraint propagation + backtracking (`propagation.cpp`)

Every cell keeps a set of candidate digits, and the solver uses logic before it guesses:

- **Naked single:** an empty cell with only one candidate left must take that digit.
- **Hidden single:** if a digit can go in only one cell of a row, column or box, it must go there, even if that cell still has other candidates.

Placing a digit removes it from the candidates of its row, column and box, which can force more cells, so this repeats until nothing changes. If a cell has no candidates left, or a digit has no place left in a unit, the board is a dead end.

When logic alone is not enough, the solver guesses a digit for the empty cell with the **fewest candidates**. Each guess is made on a copy of the board, so a wrong guess is simply discarded and the next candidate is tried.

## Build and run

Requires a C++ compiler such as `g++`.

```
g++ -std=c++17 -O2 -o bt src/backtracking.cpp
./bt

g++ -std=c++17 -O2 -o prop src/propagation.cpp
./prop
```

On Windows, run the executable as `bt.exe` (or `.\bt.exe` in PowerShell).

The puzzle is the `board` matrix in `main()`, where `0` marks an empty cell. To try another puzzle, replace the nine rows.

## Results

A **guess** is counted each time the solver tries a digit in a cell where it has to search. In the backtracking solver that is every digit that passes the row, column and box check. In the propagation solver it is every candidate tried on the cell with the fewest candidates. Cells filled by logic alone cost no guesses.

| Puzzle | Clues | Backtracking guesses | Propagation guesses | Backtracking time | Propagation time |
|---|---|---|---|---|---|
| Sample | 30 | 4208 | 0 | 0.228 ms | 0.041 ms |
| Low-clue | 23 | 13630 | 3 | 0.827 ms | 0.165 ms |
| Hard (Arto Inkala) | 21 | 49558 | 172 | 3.052 ms | 3.924 ms |
| Brute-force killer | 17 | 69175316 | 0 | 3650.678 ms | 0.11281 ms |

Puzzles as 81-digit strings (row by row, `0` = empty):

- Sample: `530070000600195000098000060800060003400803001700020006060000280000419005000080079`
- Low-clue: `000061000300400800060003700500700090040190020600000010400900500002300070000000000`
- Hard: `800000000003600000070090200050007000000045700000100030001000068008500010090000400`
- Brute-force killer: `000000000000003085001020000000507000004000100090000000500000073002010000000040009`

Times are the average of 10 runs, compiled with `-O2`, on an Intel Core i7-13650HX, Windows 11, g++ 16.2.0 (MinGW-W64).

Guess counts are the same on every machine. Times are not.

### Notes

- **Propagation needs far fewer guesses, but each guess costs more.** Every guess copies the candidate board, and every propagation pass rebuilds the row, column and box lists. On the Inkala puzzle, propagation makes about 288 times fewer guesses, but each one takes about 370 times longer (roughly 23 microseconds against 0.06), so backtracking comes out slightly faster overall.
- **Hard for humans is not the same as hard for brute force.** The Inkala puzzle needs only about 50,000 guesses from backtracking. The brute-force killer is built against a left-to-right search: backtracking needs about 69 million guesses, while propagation solves it with logic alone and no guesses.
- The planned improvements below target the per-guess overhead.

## Project structure

```
backtracking.cpp    baseline backtracking solver
propagation.cpp     constraint propagation + backtracking solver
README.md
```

## Planned improvements

Each change will be measured on its own and the table above updated.

- Stop scanning a unit as soon as a digit is found to be already placed
- Build the 27 row, column and box lists once instead of on every pass
- Keep a candidate count per cell, so single-candidate cells and dead ends are found without rescanning the board
- Replace `vector<vector<bool>>` with bitmasks for the candidate sets
