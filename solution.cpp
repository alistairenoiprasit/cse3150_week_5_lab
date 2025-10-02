#include <iostream>
#include <vector>
#include <stack>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <random>

using namespace std;

// Forward declaration
const int ROW_LENGTH = 4;
const int COL_LENGTH = 4;

int compute_score(const std::vector<std::vector<int>>& board);
std::vector<int> compress_row(const std::vector<int>& row);
std::vector<int> merge_row(std::vector<int> row);

#include <iterator>
void write_board_csv(const vector<vector<int>>& board, bool first, const string& stage) {
    ios_base::openmode mode = ios::app;
    if (first) mode = ios::trunc;
    ofstream fout("game_output.csv", mode);
    if (!fout) return;

    // Write stage identifier
    fout << stage << ",";

    // Write board data
    for (int r=0;r<4;r++){
        for (int c=0;c<4;c++){
            fout<<board[r][c];
            if (!(r==3 && c==3)) fout<<",";
        }
    }
    fout<<"\n";
}

void read_board_csv(vector<vector<int>>& board) {
    ifstream fin("game_input.csv");

    string line;
    int r = 0;
    while (getline(fin, line) && r < 4) {
        stringstream ss(line);
        string cell;
        int c = 0;
        while (getline(ss, cell, ',') && c < 4) {
            try {
                board[r][c] = stoi(cell);
            } catch (...) {
                board[r][c] = 0;  // Default to 0 for invalid input
            }
            c++;
        }
        r++;
    }
}



void print_board(const vector<vector<int>>& board) {
    // Created Sep 25
    // Modified Oct 2, removed write csv line
    for (const auto &row : board) {
        for (auto val : row) {
            if (val == 0) cout << ".\t";
            else cout << val << "\t";
        }
        cout << "\n";
    }
    cout << endl;
}

void spawn_tile(std::vector<std::vector<int>>& board) {
    std::vector<std::pair<int,int>> empty;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            if (board[r][c] == 0) empty.emplace_back(r,c);

    if (empty.empty()) return;

    static std::mt19937 gen(42);  // Fixed seed for deterministic behavior
    std::uniform_int_distribution<> pos_dist(0, empty.size()-1);
    std::uniform_int_distribution<> val_dist(1, 10);

    auto [r, c] = empty[pos_dist(gen)];
    board[r][c] = (val_dist(gen) == 1 ? 4 : 2); // 10% chance of 4
}

// Helper function :
// Runs the move algorithm for any row, or column
// new_row = vector to update
// row = initial starting data
// reverse_compressed = compressed right instead
void move(vector<int>& new_row, const vector<int>& starting_row, bool reverseCompress) {
    vector<int> temp = starting_row;
    if (reverseCompress) {
        reverse(temp.begin(), temp.end());
    }
    new_row = compress_row(temp);
    // compress(compressed, temp); removed oct2
    // depricated old function
    new_row = merge_row(new_row);
    new_row = compress_row(new_row);

    if (reverseCompress) {
        reverse(new_row.begin(), new_row.end());
    }
}

std::vector<int> compress_row(const std::vector<int>& row) {
    vector<int> compressed;
    copy_if(row.begin(), row.end(), back_inserter(compressed), [](int i){return i!=0;});
    compressed.resize(ROW_LENGTH, 0);

    return compressed;
}

std::vector<int> merge_row(std::vector<int> row) {
    // Created 25 Sep
    // Modified 2 Oct; renamed compressed->row
    for (int i = 0; i < ROW_LENGTH - 1; i++) {
        //cout << compressed[i] << "vs" << compressed[i + 1] << "\n";
        if (row[i] == row[i + 1]) {
            row[i] = row[i]*2;
            row[i+1]=0;
            i++;
        }
    }
    return row;
}

bool move_left(std::vector<std::vector<int>>& board) {
    // created sept 25
    bool moved = false;
    for (auto &row: board) {
        std::vector<int> compressed;
        vector<int> prev_row = row;

        move(compressed, row, false);

        if (compressed != prev_row) {
            // updated oct 2 , modified->moved
            moved = true;
            row.assign(compressed.begin(), compressed.end());
        }

    }
    return moved;
}

bool move_right(std::vector<std::vector<int>>& board) {
    bool moved = false;
    // createde sept 25
    for (auto &row: board) {
        vector<int> prev_row = row;

        std::vector<int> compressed;
        move(compressed, row, true);

        if (compressed != prev_row) {
            // modified oct 2 modified->moved
            moved = true;
            row.assign(compressed.begin(), compressed.end());
        }
    }
    return moved;
}

bool move_column(vector<vector<int>>& board, bool reverseCompress) {
    bool modified = false;
    for (int j=0; j<COL_LENGTH; j++) {
        std::vector<int> column;
        for (int i=0; i<ROW_LENGTH; i++) {
            column.push_back(board[i][j]);
        }
        vector<int> prev_column = column;

        std::vector<int> compressed;
        move(compressed, column, reverseCompress);
        if (compressed != prev_column) {
            modified = true;
            for (int i=0; i<ROW_LENGTH; i++) {
                board[i][j] = compressed[i];
            }
        }
    }
    return modified;
}

bool move_up(std::vector<std::vector<int>>& board) {
    bool moved = false;
    moved = move_column(board,false);

    return moved;
}

bool move_down(std::vector<std::vector<int>>& board) {
    bool moved = false;
    moved = move_column(board,true);
    return moved;
}



int compute_score(const std::vector<std::vector<int>>& board) {
    int score = 0;
    for (const auto& row : board)
        for (int val : row)
            score += val;
    return score;
}


int main(){
    vector<vector<int>> board(4, vector<int>(4,0));

    // Read initial board from CSV
    read_board_csv(board);

    stack<vector<vector<int>>> history;
    bool first=true;

    while(true){
        print_board(board);
        if (first) {
            write_board_csv(board, true, "initial");
            first = false;
        }

        cout<<"Move (w=up, a=left, s=down, d=right), u=undo, q=quit: ";
        char cmd;
        if (!(cin>>cmd)) break;
        if (cmd=='q') break;

        if (cmd=='u') {
            // If not empty:
            //   1. Set board = history.top() to restore the previous state
            //   2. Remove the top element with history.pop()
            //   3. Call print_board(board) to show the restored board
            //   4. Call write_board_csv(board, false, "undo") to log the undo
            // Use 'continue' to skip the rest of the loop iteration
            if (!history.empty()) {
                // sept 25
                board = history.top();
                history.pop();
                // oct 2
                print_board(board);
                write_board_csv(board, false, "undo");
            }
            continue;
        }

        vector<vector<int>> prev = board;
        bool moved=false;
        if (cmd=='a') moved=move_left(board);
        else if (cmd=='d') moved=move_right(board);
        else if (cmd=='w') moved=move_up(board);
        else if (cmd=='s') moved=move_down(board);

        if (moved) {
            // modified oct 2
            history.push(prev);

            // Write board after merge but before spawn
            write_board_csv(board, false, "merge");

            spawn_tile(board);
            // Write board after spawn
            write_board_csv(board, false, "spawn");
        } else {
            // No move was made
            write_board_csv(board, false, "invalid");
        }
    }
    return 0;
}
