#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <deque>
#include <tuple>
#include <vector>
#include <iostream>
using namespace std;

struct square{
    int row,col;
    
    void print_square(){
        cout << "row: " <<row << " col: " <<col<<endl;
    }
};

class board {
    public:
        int w,h; //width and height
        vector<vector<int>> true_board{};
        vector<vector<int>> revealed_board{};
        bool failure = false;
        bool gameover = false;
        
        board(int a, int b){
            w = a;
            h = b;
            initialize_board();
        }

        int check_number(int a, int b){
            int start_row,start_col,end_row,end_col,count;
            count = 0;
            start_row = a-1;
            end_row = a+1;
            start_col = b-1;
            end_col = b+1;
            if (a == 0){start_row = 0;}
            if (b == 0){start_col = 0;}
            if (a == w-1){end_row = a;}
            if (b == h-1){end_col = b;}
            for (int i = start_row;i <=end_row;i++){
                for (int j = start_col;j<=end_col;j++){
                    if (true_board[i][j] == -1){count++;}
                }
            }
            return count;
            
        }

        void plant_mines(){
            //generate mine
            srand(time(0));
            int mine_count = w*h/5;
            int row,col;
            for (int i=0;i<mine_count;i++){//there might be duplicates 
                row = rand() % w;
                col = rand() % h;
                true_board[row][col] = -1;
                
            }
        }

        void get_numbers_in_square(){
            for (int i=0;i<w;i++){
                for (int j=0;j<h;j++){
                    if (true_board[i][j]!=-1){
                        true_board[i][j] = check_number(i,j);
                    }
                    
                }
            }
        }

        void initialize_board(){
            for (int i=0;i<w;i++){
                vector<int> temp1;
                vector<int> temp2;
                true_board.push_back(temp1);
                revealed_board.push_back(temp2);
                for (int j=0;j<h;j++){
                    true_board[i].push_back(0);
                    revealed_board[i].push_back(9);
                }
                
            }
            plant_mines();
            get_numbers_in_square();
        }

        void print_board(){
            int i, j;
            printf ("    ");
            for (i=0; i<w; i++){printf ("%d  ", i);}
            printf ("\n\n");
            for (i=0; i<w; i++){
                printf ("%d   ", i);
                for (j=0; j<h; j++){
                    if (revealed_board[i][j] == 9){printf ("*  ");}
                    else{printf ("%d  ", revealed_board[i][j]);}
                }
                printf ("\n");
            }
            return;
        }

        void print_true_board(){
            int i, j;
            printf ("    ");
            for (i=0; i<w; i++){printf ("%d  ", i);}
            printf ("\n\n");
            for (i=0; i<w; i++){
                printf ("%d    ", i);
                for (j=0; j<h; j++){
                    printf ("%d  ", true_board[i][j]);
                }
                printf ("\n");
            }
            return;
        }

        square random_select_point(){
            srand(time(0));
            int random = rand() % (w*h);
            int x = random / w;
            int y = random % h;
            while (revealed_board[x][y] != 9){
                random = rand() % (w*h);
                x = random / w;
                y = random % h;
            }
            struct square point = {x,y};
            return point;
        }

        void check_gameover(){
            for (int i=0;i<w;i++){
                for (int j=0;j<h;j++){
                    if (true_board[i][j] != revealed_board[i][j]){
                        return;
                    }
                }
            }
            gameover = true;
        }
};



int main(int argc, char *argv[]){
    int width = stoi(string(argv[1]));
    int height = stoi(string(argv[2]));
    board game(width,height);
    game.print_true_board();
    deque<square> S;
    while (!(game.gameover && game.failure)){
        //game.print_board();
        int a,b;
        cout << S.empty() <<endl;
        for (deque<square>::iterator it = S.begin(); it != S.end(); ++it){it->print_square();}
        if (S.empty()){//if S empty select a random point x
            square x = game.random_select_point();
            cout << "random selected: ";
            x.print_square();
            S.push_back(x);
        }
        //cout << "step 1" <<endl;
        for (int n = 0; n < S.size(); n++){//iterate through S
            //for (deque<square>::iterator it = S.begin(); it != S.end(); ++it){
            square x = S.front();
            a = x.row;
            b = x.col;
            S.pop_front();
            //probe(x)
            int value = game.true_board[a][b];
            //cout << "value: " << value <<endl;
            game.revealed_board[a][b] = value;
            if (game.true_board[a][b] == -1){
                game.failure = true;
                game.print_true_board();
                cout << "game failed" << endl;
                return 0;
            }
            //cout << "step 2" <<endl;
            //add all the unmarked neighbors into a queue
            deque<square> unmarked_neighbors;
            int unrevealed_count, marked_count;// unrevealed count to be used in AMN, marked count to be used in AFN
            int start_row,end_row,start_col,end_col;
            unrevealed_count = 0;
            marked_count = 0;
            start_row = a-1;
            end_row = a+1;
            start_col = b-1;
            end_col = b+1;
            if (a == 0){start_row = 0;}
            if (b == 0){start_col = 0;}
            if (a == game.w-1){end_row = a;}
            if (b == game.h-1){end_col = b;}
            for (int i = start_row;i <=end_row;i++){
                for (int j = start_col;j<=end_col;j++){
                    if (game.revealed_board[i][j] == 9){
                        struct square x = {i,j};
                        unmarked_neighbors.push_back(x);
                        unrevealed_count ++;
                        }
                }
            }
            //cout << "step 3" <<endl;
            //check AFN(all free neighbors)
                //check how many mines marked around this square
            start_row = a-1;
            end_row = a+1;
            start_col = b-1;
            end_col = b+1;
            if (a == 0){start_row = 0;}
            if (b == 0){start_col = 0;}
            if (a == game.w-1){end_row = a;}
            if (b == game.h-1){end_col = b;}
            for (int i = start_row;i <=end_row;i++){
                for (int j = start_col;j<=end_col;j++){
                    if (game.revealed_board[i][j] == -1){marked_count++;}
                }
            }
            if ((marked_count - value) == 0){//if AFN, add those to S
                //cout << "AFN is true" <<endl;
                for (int n=0; n< unmarked_neighbors.size(); n++){
                    square x = unmarked_neighbors.front();
                    S.push_back(x);
                    unmarked_neighbors.pop_front();
                }
            }
            
            //check AMN(all mine neighbors)
            else if (value == unrevealed_count){//if AMN, marked them all
                //cout << "AMN is true" <<endl;
                start_row = a-1;
                end_row = a+1;
                start_col = b-1;
                end_col = b+1;
                if (a == 0){start_row = 0;}
                if (b == 0){start_col = 0;}
                if (a == game.w-1){end_row = a;}
                if (b == game.h-1){end_col = b;}
                for (int i = start_row;i <=end_row;i++){
                    for (int j = start_col;j<=end_col;j++){
                        if (game.revealed_board[i][j] == 9){
                            game.revealed_board[i][j] == -1;
                            }
                        }
                    }
            }
            game.check_gameover();
            game.print_board();
        }        
        if (game.gameover){cout << "game finished successfully"<<endl;}
    }
    return 0;
}