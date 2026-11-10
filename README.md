# Sudoku Solver in C++

Two solvers for 9x9 sudoku, written to compare a plain backtracking search with constraint propagation. Both print the solved grid, the number of guesses they needed, and the solve time, so they can be compared on the same puzzle. The propagation solver is also available as a web app.

**Live demo:** https://YOUR-SERVICE.onrender.com

The demo runs on a free plan: it sleeps when nobody is using it, so the first load can take up to a minute, and solve times are slower than in the table below, which was measured on a laptop. Guess counts are the same everywhere.

## The two approaches

### Backtracking (`backtracking/backtracking.cpp`)

The baseline. Cells are visited left to right, top to bottom. For each empty cell the solver tries the digits 1 to 9 and accepts a digit only if it does not already appear in the same row, column or 3x3 box. If the rest of the board cannot be completed with that digit, the digit is removed and the next one is tried. There are no deductions: every decision is a guess.

### Constraint propagation + backtracking (`propagation/`)

Every cell keeps a set of candidate digits, and the solver uses logic before it guesses:

- **Naked single:** an empty cell with only one candidate left must take that digit.
- **Hidden single:** if a digit can go in only one cell of a row, column or box, it must go there, even if that cell still has other candidates.

Placing a digit removes it from the candidates of its row, column and box, which can force more cells, so this repeats until nothing changes. If a cell has no candidates left, or a digit has no place left in a unit, the board is a dead end.

When logic alone is not enough, the solver guesses a digit for the empty cell with the **fewest candidates**. Each guess is made on a copy of the board, so a wrong guess is simply discarded and the next candidate is tried.

## Build and run

Requires a C++ compiler such as `g++`.

**Backtracking (command line)**

```
cd backtracking
g++ -std=c++17 -O2 -o bt backtracking.cpp
./bt
```

**Propagation (command line)**

```
cd propagation
g++ -std=c++17 -O2 -o prop propagation.cpp solver.cpp
./prop
```

On Windows, run the executables as `bt.exe` and `prop.exe` (or `.\bt.exe` in PowerShell).

The puzzle is the `board` matrix in `main()`, where `0` marks an empty cell. To try another puzzle, replace the nine rows.

## Web app

The propagation solver runs behind a small C++ web server, with a plain HTML, CSS and JavaScript frontend.

- Interactive 9x9 grid with arrow-key navigation
- Input validation: digits that clash in a row, column or box are highlighted in red, and Solve is disabled until they are fixed
- Solved digits are shown in a different colour from your clues
- Solve time (measured inside the C++ solver) and guess count are shown after each solve
- Reset (back to your puzzle) and Clear (empty grid) buttons
- Example puzzles from the table below

**Run it locally**

The server serves the `web` folder relative to where it is started, so run it from inside `propagation`.

Windows:

```
cd propagation
g++ -std=c++17 -O2 -o server server.cpp solver.cpp -lws2_32
.\server.exe
```

Linux and macOS:

```
cd propagation
g++ -std=c++17 -O2 -pthread -o server server.cpp solver.cpp
./server
```

Then open http://localhost:8080.

**API**

`POST /solve` with a JSON body. The board is 81 digits, row by row, `0` for an empty cell:

```
{"board": "530070000600195000098000060800060003400803001700020006060000280000419005000080079"}
```

Reply (200):

```
{"solved": true, "board": "<81 digits>", "guesses": 0, "timeMs": 0.05}
```

`solved` is `false` when the puzzle has no solution. Invalid puzzles (wrong length, characters other than 0-9, or a digit repeated in a row, column or box) get a 400 with a message, for example `{"error": "Row 1 has the digit 5 more than once"}`. Requests larger than 4 KB are refused.

## Deployment

The web app is deployed on Render as a Docker web service, built from `propagation/Dockerfile`. The Dockerfile expects the repository root as its build context:

```
docker build -f propagation/Dockerfile .
```

The server reads the `PORT` environment variable and listens on all interfaces when it is set. Without it, it listens on `localhost:8080` only.

## Results

A **guess** is counted each time the solver tries a digit in a cell where it has to search. In the backtracking solver that is every digit that passes the row, column and box check. In the propagation solver it is every candidate tried on the cell with the fewest candidates. Cells filled by logic alone cost no guesses.

| Puzzle | Clues | Backtracking guesses | Propagation guesses | Backtracking time | Propagation time |
|---|---|---|---|---|---|
| Sample | 30 | 4208 | 0 | 0.228 ms | 0.041 ms |
| Low-clue | 23 | 13630 | 3 | 0.827 ms | 0.165 ms |
| Hard (Arto Inkala) | 21 | 49558 | 172 | 3.052 ms | 3.924 ms |
| Brute-force killer | 17 | 69175316 | 0 | 3650.678 ms | 0.113 ms |

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
backtracking/
  backtracking.cpp    baseline backtracking solver
propagation/
  solver.h            interface of the propagation solver
  solver.cpp          constraint propagation + backtracking solver
  propagation.cpp     command-line version
  server.cpp          web server: POST /solve and the frontend files
  web/                frontend (index.html, style.css, script.js)
  httplib.h           third-party library (see Credits)
  json.hpp            third-party library (see Credits)
  Dockerfile          container build for deployment
README.md
.gitignore
```

## Credits

- [cpp-httplib](https://github.com/yhirose/cpp-httplib) by Yuji Hirose (MIT license), used for the web server
- [JSON for Modern C++](https://github.com/nlohmann/json) by Niels Lohmann (MIT license), used for reading and writing JSON

Both are single-header libraries included in the `propagation` folder.