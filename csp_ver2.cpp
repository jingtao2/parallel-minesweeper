#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <deque>
#include <tuple>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <map>
#include <random>
#include <math.h> 
#include <bitset>
#include <omp.h>
using namespace std;

int num_threads;

int rand_range(int low, int high) {
    return rand() % (high - low + 1) + low;
}

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
        // int found_mine = 0;
        // int found_non_mine = 0;
        
        board(int a, int b, int s){
            w = a;
            h = b;
            mine_count = w*h/5;//minecount = 0.2 of the number of squares in total
            cout << "mine count: "<<mine_count<<endl;
            initialize_board(s);
        }
        
        square random_select(){
            int x = rand_range(0, w - 1);
            int y = rand_range(0, h - 1);
            struct square sq = {x,y};
            while (get_curr_number(sq)!=9){
                x = rand_range(0, w - 1);
                y = rand_range(0, h - 1);
                sq = {x,y};
            }
            return sq;
        }

        void reveal(square x){//reveal a non mine square
            int i = x.row;
            int j = x.col;
            revealed_board[i][j] = true_board[i][j];
            if (revealed_board[i][j] == -1){failure = true;}
            // found_non_mine += 1;
        }

        void mark(square x){//mark a mine
            int i = x.row;
            int j = x.col;
            revealed_board[i][j] = true_board[i][j];
            if (revealed_board[i][j] != -1){failure = true;}
            // found_mine += 1;
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

        void plant_mines(int s){
            //generate mine
            srand(s);
            int row,col;
            for (int i=0;i<mine_count;i++){//there might be duplicates 
                row = rand() % w;
                col = rand() % h;
                true_board[row][col] = -1;
            }
        }

        void get_numbers_in_square(){
            #pragma omp parallel for num_threads(num_threads) collapse(2)
            for (int i=0;i<w;i++){
                for (int j=0;j<h;j++){
                    if (true_board[i][j]!=-1){
                        true_board[i][j] = check_number(i,j);
                    }   
                }
            }
        }

        void initialize_board(int s){
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
            plant_mines(s);
            get_numbers_in_square();
        }

        void print_board(bool print_true_board=false){
            int i, j;
            if (print_true_board){cout << "Printing the true game board, # stands for mines\n";}
            else{cout << "Printing the current playing game board, * stands for unrevealed squares and # stands for marked mines\n";}
            printf ("    |");
            for (i=0; i<w; i++){printf ("%d  ", i);}
            printf ("\n");
            for (i=0; i<w; i++){printf ("____");}
            printf ("\n");
            for (j=0; j<h; j++){
                // if (i < h){printf ("%d   |", i);}
                printf ("%d   |", j);
                for (i=0; i<w; i++){
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
        bool check_gameover(){
            // use mine count will be faster (have to use pointer)
            // #pragma omp parallel for num_threads(num_threads) collapse(2)
            for (int i=0;i<w;i++){
                for (int j=0;j<h;j++){
                    if (true_board[i][j] != revealed_board[i][j]){return false;}
                }    
            }
            gameover = true;
            return true;
        }
};

struct constraint{
    int value;//right hand side of the constraint equation
    vector<square> vars = {};//left hand side of the constraint equation

    bool check_satisfaction(map<square,int> sol){
        int sum = 0;
        // #pragma omp parallel for num_threads(num_threads) \
        reduction (+: sum)
        for (int i=0;i<vars.size();i++){
            sum += sol[vars[i]];
        }
        if (sum == value){
            return true;}
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

bool fully_discovered(square x,board game){
    int i = x.row;
    int j = x.col;
    if (game.revealed_board[i][j] == -1){
        return true;
    }
    start_and_end indexes = set_the_start_and_end(x.row,x.col,game.w,game.h);
    for (int a = indexes.start_row;a<=indexes.end_row;a++){
        for (int b = indexes.start_col;b<=indexes.end_col;b++){
            if (game.revealed_board[a][b] == 9){
                return false;
            }
        }
    }
    return true;
}

constraint give_constraint(square x, board game){
    struct constraint c;
    //if (game.get_true_number(x) == -1){c.value = 1;}
    c.value = game.get_curr_number(x);
    start_and_end indexes = set_the_start_and_end(x.row,x.col,game.w,game.h);
    for (int a = indexes.start_row;a<=indexes.end_row;a++){
        for (int b = indexes.start_col;b<=indexes.end_col;b++){
            struct square temp = {a,b};
            if (game.get_curr_number(temp) == 9){//which means the square is unrevealed
                c.vars.push_back(temp);
            }
            else if (game.get_curr_number(temp) == -1){
                c.value -= 1;
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
    int seed = stoi(string(argv[3]));
    int num_threads = stoi(string(argv[4]));
    board game(width,height,seed);
    //reveal the upperleft square
    struct square upperleft={0,0};
    game.reveal(upperleft);
    if (game.failure){cout << "upperleft is a mine"<<endl;}
    game.print_board(true);
    //create a vector to store constraints(which is also vectors of squares)
    vector<constraint> s;
    //update the constraints
    s.push_back(give_constraint(upperleft,game));

    //three global vars vector to store variables
    //vars for all the variables we are currently using
    //pre vars are the variables we were using in the last iteration
    //added vars are the new variables compared to pre_vars
    vector<square> vars;
    vector<square> prev_vars;
    vector<square> added_vars;
    //use a vector to store all the possible solutions for the current iteration
    vector<map<square,int>> sols;
    //store the partial solutions in this prev_sols
    //for new variables, we will add them to the solutoins we already had
    //e.g. if new variable x, we already have sols = {y=1,z=0}, then we will only try temp_sol = {x=0, y=1,z=0}, temp_sol = {x=1, y=1,z=0}
    //hence reduce the redundant work to go over the search tree every time
    vector<map<square,int>> prev_sols;
    //we need this flag because at the very beginning or everytime we encounter the situation that all the constraints are satisfied in s
    //then added_vars = vars
    bool initial_flag = true;
    //just a counter
    int loop_count =0;

    while (!(game.gameover || game.failure)){
        loop_count ++;
        cout << "loop count: " <<  loop_count <<endl;
        // for (int i=0;i<s.size();i++){
        //     s[i].print_constraint();
        // }
        //we want to know what are the new variables compared to last iteration
        added_vars = {};
        if (initial_flag){//at the very beginning, added_vars is just vars
            added_vars = related_squares(s);
            vars = added_vars;
            initial_flag = false;
        }
        else{
            vars = related_squares(s);//get all the related variables
            // #pragma omp parallel for num_threads(num_threads)
            for (int i=0;i<vars.size();i++){
                if ((find(prev_vars.begin(),prev_vars.end(),vars[i]) == prev_vars.end())){//check if every variable in vars is in prev_vars
                    square v = vars[i];
                    // #pragma omp critical
                    added_vars.push_back(v);//add the new comer into the added_vars
                }
            }
        }
        // cout << "added " << added_vars.size() << " variables" <<endl;
        // cout << "added vars contains these "<<endl;
        // for (int i=0;i<added_vars.size();i++){
        //     added_vars[i].print_square();
        //     cout << " "<<endl;
        // }
        
        //use a map to store the value assignments, we will only store assignments that we are 100% sure
        map<square,int> sol;
        //first step: if constraint = 0 or every variable is a mine, we get some deterministic solutions
        //if we have some of the values figured out, then no need to guess and pick a square
        bool guess_necessary = true;
        // #pragma omp parallel for num_threads(num_threads)
        for (int i=0;i<s.size();i++){
            //constraint = 0, all neighbors not mine
            if ((s[i].value) == 0){
                // cout << "this is a no mine neighbor constraint"<<endl;
                // s[i].print_constraint();
                guess_necessary = false;
                //add them to solution, reveal the squares on board
                for (int j=0;j<s[i].vars.size();j++){
                    // #pragma omp atomic
                    sol[s[i].vars[j]] = 0;
                    // #pragma omp critical
                    game.reveal(s[i].vars[j]);
                }
            }
            else if ((s[i].value) == s[i].vars.size()){
                //neighbor count=constraint value, all neighbors mine
                // cout << "this is a all mine neighbor constraint"<<endl;
                // s[i].print_constraint();
                guess_necessary = false;
                //add them to solution, mark the mines on board
                for (int j=0;j<s[i].vars.size();j++){
                    // #pragma omp atomic
                    sol[s[i].vars[j]] = 1;
                    // #pragma omp critical
                    game.mark(s[i].vars[j]);
                }
            }
        }
        //since we already have some solution for the added_vars
        //we want to update the added_vars
        //so that when we go into the CSP, we iterate as few iterations as possible
        vector<square> temp_vars;
        // check if variable in added_vars already exist in solution, if not, add it to temp_vars
        // no need to parallel
        for (int i=0;i<added_vars.size();i++){
            if (sol.find(added_vars[i]) == sol.end()){
                temp_vars.push_back(added_vars[i]);
            }
        }
        added_vars = temp_vars;
        //indeterministic count is the number of variables that are added in this iteration
        //and yet we dont have a deterministic value
        //so we will do CSP and find all the possible solutions and store them
        int indeterministic_count = added_vars.size();
        // cout << "indeterministic count " << indeterministic_count <<endl;
        //add new found out value in solution to prev_solutions
        // ????????????
        // #pragma omp parallel for num_threads(num_threads)
        for (int i=0;i<vars.size();i++){
            if (sol.find(vars[i]) != sol.end()){
                // #pragma omp parallel for num_threads(num_threads)
                for (int j=0;j<prev_sols.size();j++){
                    // #pragma omp atomic
                    prev_sols[j][vars[i]] = sol[vars[i]];
                }
            }
        }
        //if there is no new variable, then we dont have to do CSP to find potential solutions
        //otherwise, we always do a CSP(for speed reasons)
        //imagine we have 5 variables we don't have solutions from previous iteration
        //and now we have 5 more variables from this iteration
        //then the total assignment will be 2^10
        //if we do a CSP in the previous iteration, 2^5
        //and do a CSP based on the solutions we got from previous iteration, lets say 10 possible solutions
        //then still 2^5 + 10*2^5 << 2^10, way faster
        if (!added_vars.empty()){
            // cout << "vars contains these "<<endl;
            // for (int i=0;i<vars.size();i++){
            //         vars[i].print_square();
            //         cout << " "<<endl;
            // }
            //use for loop to find all possible solutions
            sols = {};
            int limit = pow(2,indeterministic_count);
            //each variable has two values so there are 2^n combinations
            //we loop through all the possible assignments/combinations
            #pragma omp parallel for num_threads(num_threads) schedule(dynamic,1)
            for (int k=0;k<limit;k++){
                //a progress bar
                int twentyfivepercent = 1+ (pow(2,indeterministic_count)/4);
                if (k % twentyfivepercent == 0){cout << "process: " <<k<< "/"<<pow(2,indeterministic_count)<<endl;}
                //prev_sols can be empty the first time we have an indeterministic variable
                if (prev_sols.empty()){
                    map<square,int> temp_sol;
                    //use digits of binary version of k to give assignment
                    //this ensures that each iteration, we have a different assignment, 
                    //and this will go over all the possible assignments
                    string to_binary = bitset< 64 >(k).to_string();
                    // #pragma omp parallel for num_threads(num_threads)
                    for (int i=0;i<added_vars.size();i++){
                        char a = to_binary[63-i];
                        temp_sol[added_vars[i]] = a-'0';
                        // cout << temp_sol[vars[i]] << " ";
                    }

                    bool possible_solution = true;
                    //everytime we have an possible assignment, we check it will all the constraints we have
                    #pragma omp parallel for num_threads(num_threads)
                    for (int j=0;j<s.size();j++){
                        if (!possible_solution) continue;
                        if (!s[j].check_satisfaction(temp_sol)){
                            possible_solution = false;
                            // break;
                        }//if one constraint is not satisfied
                    }
                    if (possible_solution){
                        #pragma omp critical
                        sols.push_back(temp_sol);
                    }//if possible, we find a solution, add it to sols
                }
                else{
                    //use digits of binary version of k to give assignment
                    //this ensures that each iteration, we have a different assignment, 
                    //and this will go over all the possible assignments
                    string to_binary = bitset< 64 >(k).to_string();
                    for (int p=0;p<prev_sols.size();p++){
                        //this time we have previous solutions, so we copy one of it and make assignments
                        //to the indeterministic variables
                        //all variables in the solutions in previous solutions are either deterministic ones that we just added above
                        //or possible solutions we got from previous iteration
                        map<square,int> temp_sol(prev_sols[p]);
                        // #pragma omp parallel for num_threads(num_threads)
                        for (int i=0;i<added_vars.size();i++){
                            char a = to_binary[63-i];
                            temp_sol[added_vars[i]] = a-'0';
                            // cout << temp_sol[vars[i]] << " ";
                        }
                        bool possible_solution = true;
                        #pragma omp parallel for num_threads(num_threads)
                        for (int j=0;j<s.size();j++){
                            if (!possible_solution) continue;
                            if (!s[j].check_satisfaction(temp_sol)){
                                possible_solution = false;
                                // break;
                            }//if one constraint is not satisfied
                        }
                        if (possible_solution){
                            #pragma omp critical
                            sols.push_back(temp_sol);
                        }//if not continue, we find a solution
                    }
                }
                
            }
            
            
        }
        // for (int z=0;z<sols.size();z++){
        //     cout << "potention solutions "<< z<< ": " << endl;
        //     for (int y=0;y<vars.size();y++){
        //         if (sols[z].find(vars[y]) != sols[z].end()){
        //             vars[y].print_square();
        //             cout <<  " " << sols[z][vars[y]] << endl; 
        //         }
        //     }
        // }
        //if we have to make a guess
        if (guess_necessary){
            //now we have all the possible solutions, lets calculate the probability of revealing a square and getting a mine
            //choose the square with least probability and add that to vector sol
            int number_of_sols = sols.size();
            vector<int> count_of_mine(vars.size(),0);//a vector of the count of the times this variable is a mine in all the possible solutions
            // #pragma omp parallel for num_threads(num_threads) collapse(2)
            for (int z=0;z<number_of_sols;z++){
                for (int y=0;y<vars.size();y++){
                    if (sols[z][vars[y]] == 1){
                        // #pragma omp atomic
                        count_of_mine[y] ++;
                    }
                }
            }
            int minimum_mine = number_of_sols;
            int minimum_index = -1;
            bool find_some_deterministic = false;
            // cout << "number of solutions: " <<number_of_sols <<endl;
            // count_of_mine[x] read only
            // minimum_mine and minimum_index read and write
            // sol[var[x]] read and write x
            // find_some_deterministic read and write x
            // #pragma omp parallel for num_threads(num_threads) 
            for (int x=0;x<vars.size();x++){
                // #pragma omp critical
                {
                    if (count_of_mine[x]>0 && count_of_mine[x]<minimum_mine){
                        minimum_mine = count_of_mine[x];
                        minimum_index = x;
                    }
                }
                if (count_of_mine[x] == 0){
                    if (sol.find(vars[x]) == sol.end()){
                        find_some_deterministic = true;
                        sol[vars[x]] = 0;
                        // cout << "all solution zero: ";
                        // vars[x].print_square();
                    }
                }
                if (count_of_mine[x] == number_of_sols){
                    if (sol.find(vars[x]) == sol.end()){
                        find_some_deterministic = true;
                        sol[vars[x]] = 1;
                        // cout << "all solution one: ";
                        // vars[x].print_square();
                    }
                }
            }
            if (!find_some_deterministic){
                sol[vars[minimum_index]] = 0;//we find the square with minimum likelihood of being a mine, so assign it to be non-mine
                // cout << "index is " << minimum_index << endl;
                // cout << "the least likely square is ";
                // vars[minimum_index].print_square();
                // cout <<endl;
                
            }
        }
        //copy the partial solutions into prev_sols to be used in next CSP
        prev_sols = sols;
        prev_vars = vars;
        //reveal the deterministic values and the least probability square that has a mine from vector sol
        //update the constraints
        s={};
        // #pragma omp parallel for num_threads(num_threads)
        for (int i=0;i<vars.size();i++){
            if (sol.find(vars[i]) != sol.end()){
                if (sol[vars[i]]==1){
                    // cout << "find mine ";
                    // vars[i].print_square();
                    // cout << endl;
                    // #pragma omp critical
                    game.mark(vars[i]);
                } else if (sol[vars[i]]==0){
                    // cout << "safe square ";
                    // vars[i].print_square();
                    // cout << endl;
                    // #pragma omp critical
                    game.reveal(vars[i]);
                }
            }
        }
        #pragma omp parallel for num_threads(num_threads) collapse(2)
        for (int i=0;i<game.w;i++){
            for (int j=0;j<game.h;j++){
                struct square temp_sq = {i,j};
                if (game.get_curr_number(temp_sq) != 9){
                    if (!fully_discovered(temp_sq,game)){
                        constraint c = give_constraint(temp_sq,game);
                        #pragma omp critical
                        s.push_back(c);
                    }
                }
            }
        }

        game.print_board();
        if (game.check_gameover()){
            cout << "game finished successfully"<<endl;}
        else{
            while (s.empty()){
                struct square random_sq = game.random_select();
                if (!fully_discovered(random_sq,game)){
                    s.push_back(give_constraint(random_sq,game));
                }
            }
        }
    }
    if (game.failure){cout <<"game failed"<<endl;}
    return 0;
}