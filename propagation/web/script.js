// Sudoku Solver frontend
// Sends the puzzle to the C++ server: POST /solve with {"board": "<81 digits>"}

const gridDiv = document.getElementById("grid");
const solveBtn = document.getElementById("solveBtn");
const resetBtn = document.getElementById("resetBtn");
const clearBtn = document.getElementById("clearBtn");
const examplesSelect = document.getElementById("examples");
const messageDiv = document.getElementById("message");
const statsDiv = document.getElementById("stats");
const statTime = document.getElementById("statTime");
const statGuesses = document.getElementById("statGuesses");

// example puzzles (81 digits, row by row, 0 = empty)
const examples = {
    sample: "530070000600195000098000060800060003400803001700020006060000280000419005000080079",
    lowclue: "000061000300400800060003700500700090040190020600000010400900500002300070000000000",
    hard: "800000000003600000070090200050007000000045700000100030001000068008500010090000400",
    killer: "000000000000003085001020000000507000004000100090000000500000073002010000000040009"
};

let cells = [];          // the 81 input boxes, row by row
let units = [];          // the 27 rows, columns and boxes (each is a list of 9 cell numbers)
let savedPuzzle = "";    // the puzzle as entered before solving, used by Reset
let isSolved = false;


// creates the 81 input boxes
function buildGrid(){
    for(let i=0; i<81; i++){
        const row = Math.floor(i / 9);
        const col = i % 9;

        const cell = document.createElement("input");
        cell.type = "text";
        cell.inputMode = "numeric";
        cell.autocomplete = "off";
        cell.className = "cell";
        cell.setAttribute("aria-label", "Row " + (row + 1) + ", column " + (col + 1));

        // thicker lines between the 3x3 boxes
        if(col === 2 || col === 5){
            cell.classList.add("right-border");
        }
        if(row === 2 || row === 5){
            cell.classList.add("bottom-border");
        }

        cell.addEventListener("input", function(){ onInput(i); });
        cell.addEventListener("keydown", function(e){ onKey(e, i); });
        cell.addEventListener("focus", function(){ cell.select(); });

        gridDiv.appendChild(cell);
        cells.push(cell);
    }
}

// lists the cell numbers of every row, column and 3x3 box
function buildUnits(){
    for(let i=0; i<9; i++){
        const row = [];
        const col = [];
        const box = [];
        for(let j=0; j<9; j++){
            row.push(i*9 + j);
            col.push(j*9 + i);
            box.push((Math.floor(i/3)*3 + Math.floor(j/3))*9 + (i%3)*3 + j%3);
        }
        units.push(row);
        units.push(col);
        units.push(box);
    }
}

// returns the grid as a list of 81 numbers, 0 = empty
function getBoard(){
    const board = [];
    for(let i=0; i<81; i++){
        if(cells[i].value === ""){
            board.push(0);
        } else {
            board.push(parseInt(cells[i].value));
        }
    }
    return board;
}

// fills the grid from an 81-character string and unlocks it
function setBoard(str){
    for(let i=0; i<81; i++){
        if(str[i] === "0"){
            cells[i].value = "";
        } else {
            cells[i].value = str[i];
        }
        cells[i].readOnly = false;
        cells[i].classList.remove("solved-digit");
    }
    isSolved = false;
    resetBtn.disabled = true;
    hideStats();
}

// returns the cell numbers that break the rule (a digit twice in a row, column or box)
function checkConflicts(){
    const board = getBoard();
    const bad = new Set();

    for(let u=0; u<units.length; u++){
        for(let d=1; d<=9; d++){
            const where = [];
            for(let k=0; k<9; k++){
                if(board[units[u][k]] === d){
                    where.push(units[u][k]);
                }
            }
            if(where.length > 1){
                for(let w=0; w<where.length; w++){
                    bad.add(where[w]);
                }
            }
        }
    }
    return bad;
}

// marks clashing cells in red and turns the Solve button on or off
function updateValidation(){
    const bad = checkConflicts();

    for(let i=0; i<81; i++){
        cells[i].classList.toggle("conflict", bad.has(i));
    }

    if(bad.size > 0){
        showMessage("Some numbers clash (shown in red). A digit can appear only once in each row, column and 3x3 box.", "error");
        solveBtn.disabled = true;
    } else {
        showMessage("", "");
        solveBtn.disabled = isSolved;
    }
}

function showMessage(text, type){
    messageDiv.textContent = text;
    messageDiv.className = "message " + type;
}

function showStats(data){
    statTime.textContent = data.timeMs.toFixed(3) + " ms";
    statGuesses.textContent = data.guesses;
    statsDiv.classList.remove("hidden");
}

function hideStats(){
    statsDiv.classList.add("hidden");
}


// typing: keep one digit 1-9, anything else is removed
function onInput(i){
    const value = cells[i].value.replace(/[^1-9]/g, "");
    cells[i].value = value.slice(-1);
    hideStats();
    updateValidation();
}

// arrow keys move between cells, Enter solves
function onKey(e, i){
    const row = Math.floor(i / 9);
    const col = i % 9;
    let target = -1;

    if(e.key === "ArrowLeft" && col > 0){
        target = i - 1;
    }
    else if(e.key === "ArrowRight" && col < 8){
        target = i + 1;
    }
    else if(e.key === "ArrowUp" && row > 0){
        target = i - 9;
    }
    else if(e.key === "ArrowDown" && row < 8){
        target = i + 9;
    }

    if(target !== -1){
        e.preventDefault();
        cells[target].focus();
    }
    else if(e.key === "Enter" && !solveBtn.disabled){
        solve();
    }
}


// sends the puzzle to the server and shows the answer
async function solve(){
    const puzzle = getBoard().join("");
    savedPuzzle = puzzle;

    solveBtn.disabled = true;
    solveBtn.textContent = "Solving...";

    try{
        const response = await fetch("/solve", {
            method: "POST",
            headers: {"Content-Type": "application/json"},
            body: JSON.stringify({board: puzzle})
        });
        const data = await response.json();

        if(!response.ok){
            showMessage(data.error, "error");
        }
        else if(!data.solved){
            showMessage("This puzzle has no solution. Check the numbers you entered.", "error");
            showStats(data);
        }
        else{
            showSolution(data);
        }
    }
    catch(err){
        showMessage("Could not reach the server. Is it running?", "error");
    }

    solveBtn.textContent = "Solve";
    if(!isSolved){
        solveBtn.disabled = false;
    }
}

// fills the empty cells with the solver's digits and locks the grid
function showSolution(data){
    for(let i=0; i<81; i++){
        if(cells[i].value === ""){
            cells[i].value = data.board[i];
            cells[i].classList.add("solved-digit");
        }
        cells[i].readOnly = true;
    }
    isSolved = true;
    solveBtn.disabled = true;
    resetBtn.disabled = false;
    showMessage("Puzzle solved. The digits found by the solver are shown in colour.", "success");
    showStats(data);
}

// back to the puzzle as it was entered, without the solver's digits
function resetPuzzle(){
    setBoard(savedPuzzle);
    updateValidation();
}

// empty grid
function clearGrid(){
    setBoard("0".repeat(81));
    savedPuzzle = "";
    updateValidation();
    cells[0].focus();
}

function loadExample(){
    const key = examplesSelect.value;
    if(key === ""){
        return;
    }
    setBoard(examples[key]);
    savedPuzzle = "";
    updateValidation();
    examplesSelect.value = "";
}


solveBtn.addEventListener("click", solve);
resetBtn.addEventListener("click", resetPuzzle);
clearBtn.addEventListener("click", clearGrid);
examplesSelect.addEventListener("change", loadExample);

buildUnits();
buildGrid();
setBoard("0".repeat(81));
updateValidation();