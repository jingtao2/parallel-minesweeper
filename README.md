# parallel-minesweeper
The code is written in C++. Please compile and execute:
w = width of the board
h = height of the board
s = seed number
n = number of threads

csp_seq.cpp:
	g++ -g -w -std=c++11 -o ms csp_seq.cpp
    ./ms w h s
csp_ver1.cpp
	g++ -g -w -fopenmp -std=c++11 -o ms1 csp_ver1.cpp
    ./ms1 w h s
csp_ver2.cpp
	g++ -g -w -fopenmp -std=c++11 -o ms2 csp_ver2.cpp
    ./ms2 w h s n

Since the board generates mine positions with srand command, the board generated will be different and the execution time can be totally different.
For a 30*16 board, the execution time is roughly in the range of 20 to 120 seconds for a successful solve.
The success rate for a Constraint Satisfaction Problem strategy solver on the 30*16 board is around 30% (which is state of the art).
So if the solver fails, please try a different seed.
