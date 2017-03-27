/*
Name: Andrew W Markley
BlazerId: amarkley
Course Section: CS 432
Homework #: 2
*/

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <zconf.h>
#include <fstream>
#include <cmath>
#include "mpi.h"

using namespace std;

bool const DEBUG = true;

double gettime(void) {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

class GameOfLife {
public:
    //Default constructor uses a 10x10 matrix
    GameOfLife(int size) {
        generateBoard(size);
    }

    //Generates a board of dimensions size x size and seeds the board with a random distribution of
    //cells to prepare for the first turn.
    void generateBoard(int size) {
        N = size;
        double chance = 0.3;

        currentBoard = new bool [N * N];
//        nextBoard.resize(N*N,0);

        srand48(1);

        for ( int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                currentBoard[i * N + j] = (bool) (drand48() < chance );
            }
        }
    }

    //Prints a filled square for every cell that is marked "alive" in the current turn.
    void printBoard() {
        cout << string(N*3, '-') << endl;

        for ( int i = 0; i < N; i++){
            for (int j = 0; j < N; j++){
                string s = " . ";

                if ( currentBoard[i * N + j] )
                    s = " ■ ";

                cout << s;
            }
            cout << endl;
        }
    }

    void printBoard( bool *board, int size, int boardLength) {

        cout << string(size*3, '-') << endl;

        for ( int i = 0; i < boardLength / size ; i++){
            for (int j = 0; j < size; j++){
                string s = " . ";

                if ( board[i * N + j] )
                    s = " ■ ";

                cout << s;
            }
            cout << endl;
        }
    }

    pair<int,int> getBounds( int threadID ) {
        int maxThreads;
        MPI_Comm_size(MPI_COMM_WORLD, &maxThreads);

        int remainder = N % maxThreads;
        int span = N / maxThreads;

        int start = threadID * span;
        int end = (threadID + 1 ) * span;

        if ( threadID == maxThreads - 1 )
            end = N;

        return make_pair(start, end);
    }

    int getChunk( int threadID ) {
        pair<int,int> bounds = getBounds( threadID );
        return bounds.second - bounds.first;
    }

    int countNeighbors(int i, int j) {
        int count = 0;

        //Check bounds
        int left = j-1;
        int right = j+1;
        int top = i-1;
        int bottom = i+1;

        //Check top row, then left and right cells in top
        if ( top >= 0 ) {
            count += currentBoard[top*N+j];
            if (left >= 0)
                count += currentBoard[top * N + left];
            if (right < N)
                count += currentBoard[top * N + right];
        }

        //Check bottom row, then left and right cells in bottom
        //Includes optimization for cases in which there are already more than 3 neighbors
        if ( bottom < N) {
            count += currentBoard[bottom * N + j];
            if (left >= 0 && count < 4)
                count += currentBoard[bottom * N + left];
            if (right < N && count < 4)
                count += currentBoard[bottom * N + right];
        }

        //Check left cell
        if (left >= 0 && count < 4)
            count += currentBoard[i * N + left];

        //Check right cell
        if (right < N && count < 4)
            count += currentBoard[i * N + right];

        return count;
    }

    //Calculates the changes for the next turn of the board
//    void nextTurn( bool *result ) {
//        int my_id;
//        MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
//
//        pair<int,int> range = getBounds(my_id);
//        bool my_result[ getChunk(my_id) * N ];
//
//        int my_position = 0;
//
//        for ( int i = range.first; i < range.second; i++) {
//            for ( int j = 0; j < N; j++) {
//
//                int count = countNeighbors(i, j);
//
//                /////////////////////////////////////////////////////////////////////////////////
//                //Determine if a cell lives or dies
//                //Living cell behavior:
//                if ( currentBoard[i*N+j] ) {
//                    //Less than 2 neighbors or more than 3 neighbors, the cell dies
//                    if ( count < 2 || count > 3)
//                       my_result[ my_position++ ] = 0;     //set(i,j,0);
//                    else
//                        my_result[ my_position++] = 1;
//                }
//                    //Cell is dead, becomes living if there are 3 neighbors
//                else if ( count == 3 )
//                    my_result[ my_position++ ] = 1;     //(i,j,1);
//                    //Otherwise, no change
//                else
//                    my_result[ my_position++ ] = 0;      //set(i,j,0);
//                /////////////////////////////////////////////////////////////////////////////////
//            }
//        }
//
//        printBoard( my_result, N , sizeof(my_result)/sizeof(my_result[0]));
//
//        memcpy( result, my_result, sizeof(result));
//
//    }


    double run(int rounds) {

        //// Begin multithreading
        MPI_Init(NULL, NULL);

        //// Begin setup
        double start = MPI_Wtime();
        int numThreads, my_rank;
        bool *my_board;

        MPI_Comm_size(MPI_COMM_WORLD, &numThreads);
        MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

        int chunks[numThreads];
        int displacement[numThreads];

        if ( my_rank == 0 ) {
            my_board = currentBoard;
//            printBoard();
        }
        else {
            my_board = new bool [N * N];
        }

        MPI_Bcast( my_board, N * N, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);

        for ( int i = 0; i < numThreads; i++ ) {
            chunks[i] = getChunk(i) * N;
            displacement[i] = 0;
        }

        bool *nextBoard = new bool [N * N];
        bool *my_result = new bool [ chunks[my_rank] ];

        /////////////////////////////////////////////////////////////////////////////////
        //// Begin running rounds
        for ( int i = 0; i < rounds; i++ ) {

            pair<int,int> range = getBounds(my_rank);

            int my_position = 0;

            for ( int i = range.first; i < range.second; i++) {
                for ( int j = 0; j < N; j++) {

                    int count = countNeighbors(i, j);

                    /////////////////////////////////////////////////////////////////////////////////
                    //Determine if a cell lives or dies
                    //Living cell behavior:
                    if ( currentBoard[i*N+j] ) {
                        //Less than 2 neighbors or more than 3 neighbors, the cell dies
                        if ( count < 2 || count > 3)
                            my_result[ my_position++ ] = 0;     //set(i,j,0);
                        else
                            my_result[ my_position++] = 1;
                    }
                        //Cell is dead, becomes living if there are 3 neighbors
                    else if ( count == 3 )
                        my_result[ my_position++ ] = 1;     //(i,j,1);
                        //Otherwise, no change
                    else
                        my_result[ my_position++ ] = 0;      //set(i,j,0);
                    /////////////////////////////////////////////////////////////////////////////////
                }
            }

            MPI_Allgatherv( my_result, chunks[my_rank], MPI_CXX_BOOL,
                            nextBoard, chunks, displacement, MPI_CXX_BOOL, MPI_COMM_WORLD);


            endTurn( nextBoard );

            if ( DEBUG && my_rank == 0 ) {
//                printBoard();
            }

        }
        //// End running rounds
        /////////////////////////////////////////////////////////////////////////////////

        delete[] nextBoard;
        delete[] my_result;

        double end = MPI_Wtime();
        MPI_Finalize();

        if ( my_rank == 0 ) {
            cout << end - start << " seconds" << endl;

            ofstream output;
            output.open("output.log", ios::out | ios::app);

            output << numThreads << " threads: " << N << " x " << N << " for " << rounds << " rounds took "
                   << end - start << " seconds" << endl;

            output.close();
        }

        return end - start;

    }


private:
    bool *currentBoard;
    int N;

    //Sets the next board to be the current board
    void endTurn( bool *next ) {
        memcpy( currentBoard, next, sizeof(currentBoard)) ;
    }
};


int main( int numargs, char *args[]) {

    int size, rounds;


    size = atoi( args[1] );
    rounds = atoi( args[2] );

    GameOfLife life(size);

    //Begin game
    life.run(rounds);



    return 0;
}