/*
Name: Andrew W Markley
BlazerId: amarkley
Course Section: CS 432
Homework #: 4
*/

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <zconf.h>
#include <fstream>
#include <omp.h>

using namespace std;

typedef vector<vector<bool> > board;

double gettime(void) {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

class GameOfLife {
public:
    //Default constructor uses a 10x10 matrix
    GameOfLife(int size, int numThreads) {
        maxThreads = numThreads;
        omp_set_num_threads(numThreads);
        generateBoard(size);
    }

    //Generates a board of dimensions size x size and seeds the board with a random distribution of
    //cells to prepare for the first turn.
    void generateBoard(int size) {
        N = size;
        double chance = 0.3;

        currentBoard.resize(N, vector<bool>(N,0));
        nextBoard.resize(N, vector<bool>(N,0));

        srand48(time(0));

#    pragma omp parallel for
        for ( int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                currentBoard[i][j] = (bool) (drand48() < chance );
            }
        }
    }

    //Prints a filled square for every cell that is marked "alive" in the current turn.
    void printBoard() {
        cout << string(20, '\n');

        for ( int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                string s = "   ";

                if ( currentBoard[i][j] )
                    s = " ■ ";

                cout << s;
            }
            cout << endl;
        }
    }
    
    //Calculates the changes for the next turn of the board
    void nextTurn( ) {

#    pragma omp parallel for
        for ( int i = 0; i < N; i++) {
            for ( int j = 0; j < N; j++) {
                int count = 0;

                //Check bounds
                int left = j-1;
                int right = j+1;
                int top = i-1;
                int bottom = i+1;

                //Check top row, then left and right cells in top
                if ( top >= 0 ) {
                    count += currentBoard[top][j];
                    if (left >= 0)
                        count += currentBoard[top][left];
                    if (right < N)
                        count += currentBoard[top][right];
                }

                //Check bottom row, then left and right cells in bottom
                //Includes optimization for cases in which there are already more than 3 neighbors
                if ( bottom < N) {
                    count += currentBoard[bottom][j];
                    if (left >= 0 && count < 4)
                        count += currentBoard[bottom][left];
                    if (right < N && count < 4)
                        count += currentBoard[bottom][right];
                }

                //Check left cell
                if (left >= 0 && count < 4)
                    count += currentBoard[i][left];

                //Check right cell
                if (right < N && count < 4)
                    count += currentBoard[i][right];


                /////////////////////////////////////////////////////////////////////////////////
                //Determine if a cell lives or dies
                //Living cell behavior:
                if ( currentBoard[i][j] ) {
                    //Less than 2 neighbors or more than 3 neighbors, the cell dies
                    if ( count < 2 || count > 3)
                        set(i,j,0);
                }
                //Cell is dead, becomes living if there are 3 neighbors
                else if ( count == 3 )
                    set(i,j,1);
                //Otherwise, no change
                else
                    set(i,j,0);
                /////////////////////////////////////////////////////////////////////////////////
            }
        }
        endTurn();

    }


private:
    board currentBoard;
    board nextBoard;
    int N;
    int maxThreads;
    
    //Helper function to set values on the next turn's board.
    void set(int x, int y, bool alive) {
        nextBoard[x][y] = alive;
    }

    //Sets the next board to be the current board
    void endTurn() {
        currentBoard = nextBoard;
    }
};


int main( int numargs, char *args[]) {

    double starttime, endtime;
    int size, rounds, threads;


    size = atoi( args[1] );
    rounds = atoi( args[2] );
    threads = atoi( args[3] );

    GameOfLife life(size, threads);

    starttime = gettime();

//    life.printBoard();


    for ( int i = 0; i < rounds; i++ ) {
//
//        if ( i % 100 == 0 )
//            cout << i << endl;

        life.nextTurn();

//      life.printBoard();
//      sleep(1);
    }

    endtime = gettime();

    ofstream output;
    output.open( "output.log", ios::out | ios::app );

    cout << size << " x " << size << " for " << rounds << " rounds took " << endtime - starttime << " seconds" << endl;

    output << size << " x " << size << " for " << rounds << " rounds took " << endtime - starttime << " seconds" << endl;

    output.close();

    return 0;
}
