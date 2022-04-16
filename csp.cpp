#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <deque>
#include <tuple>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <map>
#include <random>
#include <math.h> 
using namespace std;

struct square{//a way to represent a square on the board
    int row,col;
    
    void print_square(){
        cout << "{" <<row << ", "<< col <<"}";
    }
    bool operator<( const square& other) const
    {
        if ( row == other.row ){
            return col < other.col;
        }
        return row < other.row;
    }
    
    bool operator==(const square& other) const
    {return(row==other.row && col==other.col);}
};

struct start_and_end{//a way to investigate the 8 square neighbors of a given square
    int start_row,start_col,end_row,end_col;
};
//return the indexes of the 8 (or less) neighbors
start_and_end set_the_start_and_end(int a, int b, int w, int h){
    struct start_and_end temp;
    temp.start_row = a-1;
    temp.end_row = a+1;
    temp.start_col = b-1;
    temp.end_col = b+1;
    if (a == 0){temp.start_row = 0;}
    if (b == 0){temp.start_col = 0;}
    if (a == w-1){temp.end_row = a;}
    if (b == h-1){temp.end_col = b;}
    return temp;
}

class board {
    public:
        int w,h; //width and height
        int mine_count;//minecount = 0.2 of the number of squares in total
        vector<vector<int>> true_board{};//the true board
        vector<vector<int>> revealed_board{};//the current game board
        bool failure = false;
        bool gameover = false;
        
        board(int a, int b){
            w = a;
            h = b;
            mine_count = w*h/5;//minecount = 0.2 of the number of squares in total
            initialize_board();
        }

        void reveal(square x){//reveal a non mine square
            int i = x.row;
            int j = x.col;
            revealed_board[i][j] = true_board[i][j];
            if (revealed_board[i][j] == -1){failure = true;}
        }

        void mark(square x){//mark a mine
            int i = x.row;
            int j = x.col;
            revealed_board[i][j] = true_board[i][j];
            if (revealed_board[i][j] != -1){failure = true;}
        }

        int check_number(int a, int b){
            struct start_and_end temp = set_the_start_and_end(a,b,w,h);
            int count = 0;
            for (int i = temp.start_row;i <=temp.end_row;i++){
                for (int j = temp.start_col;j<=temp.end_col;j++){
                    if (true_board[i][j] == -1){count++;}
                }
            }
            return count;
        }

        void plant_mines(){
            //generate mine
            srand(time(0));
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

        void print_board(bool print_true_board=false){
            int i, j;
            if (print_true_board){cout << "Printing the true game board, # stands for mines\n";}
            else{cout << "Printing the current playing game board, * stands for unrevealed squares and # stands for marked mines\n";}
            printf ("    |");
            for (i=0; i<w; i++){printf ("%d  ", i);}
            printf ("\n");
            for (i=0; i<w; i++){printf ("____", i);}
            printf ("\n");
            for (i=0; i<w; i++){
                printf ("%d   |", i);
                for (j=0; j<h; j++){
                    if (print_true_board){
                        if (true_board[i][j]==-1){
                            printf("#  ");
                        }
                        else{printf ("%d  ", true_board[i][j]);}}
                    else{
                        if (revealed_board[i][j] == 9){printf ("*  ");}//* in graph is unrevealed square
                        else if(revealed_board[i][j] == -1){printf ("#  ");}//# in graph is marked mine
                        else{printf ("%d  ", revealed_board[i][j]);}
                    }
                }
                printf ("\n");
            }
            return;
        }

        int get_curr_number(square x){//a get function to get the number of a square in revealed board fast
            int i = x.row;
            int j = x.col;
            return revealed_board[i][j];
        }
        int get_true_number(square x){//a get function to get the number of a square in true board fast
            int i = x.row;
            int j = x.col;
            return true_board[i][j];
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

struct constraint{
    int value;//right hand side of the constraint equation
    vector<square> vars = {};//left hand side of the constraint equation

    bool check_satisfaction(map<square,int> sol){
        int sum = 0;
        for (int i=0;i<vars.size();i++){
            vars[i].print_square();
            cout << " " << sol[vars[i]];
            sum += sol[vars[i]];
        }
        cout << " = " <<value <<endl;
        if (sum == value){return true;}
        return false;
    }
    void print_constraint(){
        for (int i=0;i<vars.size();i++){
            vars[i].print_square();

            if (i != (vars.size()-1)){cout << " + ";}
        }
        cout << " = " << value <<endl;
    }
};

bool isDuplicate(constraint t1, constraint t2){//this only works for the same permutation of vectors
    if (t1.value == t2.value){
        if (t1.vars == t2.vars){
            return true;
        }
    } 
    return false;
}

constraint give_constraint(square x, board game){
    struct constraint c;
    //if (game.get_true_number(x) == -1){c.value = 1;}
    c.value = game.get_true_number(x);
    start_and_end indexes = set_the_start_and_end(x.row,x.col,game.w,game.h);
    for (int a = indexes.start_row;a<=indexes.end_row;a++){
        for (int b = indexes.start_col;b<=indexes.end_col;b++){
            struct square temp = {a,b};
            if (game.get_curr_number(temp) == 9){//which means the square is unrevealed
                c.vars.push_back(temp);
            }
        }
    }
    return c;
}

vector<square> related_squares(vector<constraint> s){//give us the vector of related squares of the current constraints
    vector<square> related = {};
    for (int i=0;i<s.size();i++){
        for (int j=0;j<s[i].vars.size();j++){
            if (find(related.begin(),related.end(),s[i].vars[j])==related.end()){//if not find, add the square to the vector
                related.push_back(s[i].vars[j]);
            }
        }
    }
    return related;
}

//first step: always reveal the upperleft square
//update the constraints
//while game not finished keep going
////deterministic step: check constraints, if there are ones we know easily
////non deterministic step: go through the whole search tree to find solution(s) of the current constraints
////update the constraint
int main(int argc, char *argv[]){
    //read in inputs, initiate the board and the game
    int width = stoi(string(argv[1]));
    int height = stoi(string(argv[2]));
    board game(width,height);
    //reveal the upperleft square
    struct square upperleft={0,0};
    game.reveal(upperleft);
    game.print_board(true);
    //create a vector to store constraints(which is also vectors of squares)
    vector<constraint> s = {};
    //update the constraints
    s.push_back(give_constraint(upperleft,game));
    if (s[0].value == -1){
        game.failure = true;
        cout << "upperleft is a mine"<<endl;
    }
    int loop_count =0;
    while (!(game.gameover || game.failure)){
        loop_count ++;
        //cout << "loop count: " <<  loop_count <<endl;
        for (int i=0;i<s.size();i++){
            s[i].print_constraint();
        }
        vector<square> vars = related_squares(s);//get the related variables ready for CSP
        //use a vector to store all the possible solutions
        vector<map<square,int>> sols;
        //use a map to store the value assignments
        map<square,int> sol;
        //first step: if constraint = 0 or every variable is a mine, we get some deterministic solutions
        for (int i=0;i<s.size();i++){
            if ((s[i].value) == 0){//constraint = 0, all neighbors not mine
                for (int j=0;j<s[i].vars.size();j++){
                    sol[s[i].vars[j]] = 0;
                    game.reveal(s[i].vars[j]);
                }
            }
            if ((s[i].value) == s[i].vars.size()){//neighbor count=constraint value, all neighbors mine
                for (int j=0;j<s[i].vars.size();j++){
                    sol[s[i].vars[j]] = 1;
                    game.mark(s[i].vars[j]);
                }
            }
        }
        //if we have all the values for all the variables, we are done, no need to CSP
        bool csp_necessary = false;
        for (int i=0;i<s.size();i++){
            if (sol.find(vars[i]) == sol.end()){
                csp_necessary = true;
                break;
            }
        }
        
        if (csp_necessary){
            //use for loop to find all possible solutions
            bool possible_solution;
            for (int k=0;k<pow(2,vars.size());k++){//each variable has two values so there are 2^n combinations
                map<square,int> temp_sol = sol.copy();//we copy a solution so we have the deterministic solutions and the non determined ones everytime
                //we will use binary number to assign values for example for n=3, 2^3=8, 8 in binary is 111 then we take all the values to be 1, if k=5=101 then take 101
                for (int i=0;i<vars.size();i++){
                    if (temp_sol.find(vars[i]) == temp_sol.end()) {//check if this key already exists and if not exist, we assign a value to the variable               
                        temp_sol[vars[i]] = (k/(i+1)) % 2;
                    }
                }
                possible_solution = true;
                for (int j=0;j<s.size();j++){
                    if (!s[j].check_satisfaction(temp_sol)){
                        continue_flag = false;
                        break;}//if one constraint is not satisfied
                }
                if (possible_solution){sols.push_back(temp_sol);}//if not continue, we find a solution
            }
            //now we have all the possible solutions, lets calculate the probability of revealing a square and getting a mine
            //choose that square and add that to vector sol
            //TBD
        }
        //reveal the deterministic values and the least probability square that has a mine from vector sol
        //update the constraints
        s={};
        for (int i=0;i<vars.size();i++){
            if (sol[vars[i]]==1){game.mark(vars[i]);}
            else if (sol[vars[i]]==0){
                game.reveal(vars[i]);
                s.push_back(give_constraint(vars[i],game));
            }
        }

        if (game.gameover){cout << "game finished successfully"<<endl;}
    }
    if (game.failure){cout <<"game failed"<<endl;}
    return 0;
}