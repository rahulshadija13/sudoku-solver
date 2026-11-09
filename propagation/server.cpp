/*
 * Sudoku web server
 *
 * POST /solve   body: {"board": "<81 digits, 0 = empty>"}
 *   200 -> {"solved": true/false, "board": "<81 digits>", "guesses": n, "timeMs": t}
 *   400 -> {"error": "<what is wrong with the request or the puzzle>"}
 * Everything else is served from the web/ folder (the frontend).
 *
 * Build (Windows): g++ -std=c++17 -O2 -o server server.cpp solver.cpp -lws2_32
 * Run:             ./server   then open http://localhost:8080
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include "httplib.h"
#include "json.hpp"
#include "solver.h"
using namespace std;
using json = nlohmann::json;

// Turns the 81-character string into a 9x9 grid
vector<vector<int>> toBoard(const string& s){
    vector<vector<int>> board(9, vector<int>(9, 0));
    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            board[i][j] = s[i*9 + j] - '0';
        }
    }
    return board;
}

// Turns a 9x9 grid back into an 81-character string
string toString(const vector<vector<int>>& board){
    string s = "";
    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            s += to_string(board[i][j]);
        }
    }
    return s;
}

// Returns "" if the puzzle string is fine, otherwise a message saying what is wrong
string validatePuzzle(const string& s){
    if(s.size() != 81){
        return "Puzzle must have exactly 81 characters";
    }

    for(int i=0; i<81; i++){
        if(s[i] < '0' || s[i] > '9'){
            return "Puzzle can only contain the digits 0-9 (0 = empty cell)";
        }
    }

    // a digit can appear only once in every row, column and box
    for(int i=0; i<9; i++){
        vector<bool> rowSeen(10, false);
        vector<bool> colSeen(10, false);
        vector<bool> boxSeen(10, false);

        for(int j=0; j<9; j++){
            int r = s[i*9 + j] - '0';
            int c = s[j*9 + i] - '0';
            int b = s[((i/3)*3 + j/3)*9 + (i%3)*3 + j%3] - '0';

            if(r != 0){
                if(rowSeen[r]){
                    return "Row " + to_string(i+1) + " has the digit " + to_string(r) + " more than once";
                }
                rowSeen[r] = true;
            }
            if(c != 0){
                if(colSeen[c]){
                    return "Column " + to_string(i+1) + " has the digit " + to_string(c) + " more than once";
                }
                colSeen[c] = true;
            }
            if(b != 0){
                if(boxSeen[b]){
                    return "Box " + to_string(i+1) + " has the digit " + to_string(b) + " more than once";
                }
                boxSeen[b] = true;
            }
        }
    }

    return "";
}

// Sends a 400 reply with an error message
void sendError(httplib::Response& res, const string& message){
    json reply;
    reply["error"] = message;
    res.status = 400;
    res.set_content(reply.dump(), "application/json");
}

int main(){
    httplib::Server svr;

    // a puzzle is only about 100 bytes, so refuse anything bigger than 4 KB
    svr.set_payload_max_length(4096);

    svr.Post("/solve", [](const httplib::Request& req, httplib::Response& res){
        // false = don't throw if the body is not valid JSON
        json body = json::parse(req.body, nullptr, false);

        if(body.is_discarded() || !body.is_object() || !body.contains("board") || !body["board"].is_string()){
            sendError(res, "Request must be JSON like {\"board\": \"<81 digits>\"}");
            return;
        }

        string puzzle = body["board"].get<string>();
        string err = validatePuzzle(puzzle);
        if(err != ""){
            sendError(res, err);
            return;
        }

        SolveResult result = solveSudoku(toBoard(puzzle));

        json reply;
        reply["solved"] = result.solved;
        reply["board"] = toString(result.board);
        reply["guesses"] = result.guesses;
        reply["timeMs"] = result.timeMs;
        res.set_content(reply.dump(), "application/json");
    });

    // the frontend (step 3) goes into the web folder
    if(!svr.set_mount_point("/", "./web")){
        cout << "Note: no web folder found yet (the frontend comes next)" << endl;
    }

    // hosting platforms give the port through the PORT variable
    int port = 8080;
    string host = "127.0.0.1";      // only your own PC can connect
    const char* envPort = getenv("PORT");
    if(envPort != nullptr){
        port = atoi(envPort);
        host = "0.0.0.0";           // reachable from outside the container
    }

    cout << "Server running at http://localhost:" << port << endl;
    if(!svr.listen(host, port)){
        cout << "Could not start the server (is port " << port << " already in use?)" << endl;
    }
    return 0;
}