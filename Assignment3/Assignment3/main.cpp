/*
Name: Andrew W Markley
BlazerId: amarkley
Course Section: CS 432
Homework #: 3
 
 Compile with:
 icc -std=c++1y -O3 main.cpp [Board Size] [# of Rounds] [# of Threads]
*/

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <zconf.h>
#include <fstream>
#include <pthread.h>

class GameOfLife;

class MyThread {
public:
    MyThread();
    void set(int _argument, GameOfLife* gol);
    void start();
    void doThings(GameOfLife* x, int threadNum);
    static void *threadMethod(void *arg);
    GameOfLife* getGame() const;
    int getThread() const;
    void kill();
private:
    pthread_t mThreadId;
    int mArgument;
    GameOfLife* life;
};

using namespace std;

typedef vector<vector<bool> > board;

double gettime(void) {
    struct timeval tval;

    gettimeofday(&tval, NULL);

    return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}



class GameOfLife {
public:
    GameOfLife(int size, int _maxThreads) {
        generateBoard(size);
        maxThreads = _maxThreads;
        threadList = new MyThread[_maxThreads];
    }

    //Generates a board of dimensions size x size and seeds the board with a random distribution of
    //cells to prepare for the first turn.
    void generateBoard(int size) {
        N = size;
        double chance = 0.3;

        currentBoard.resize(N, vector<bool>(N,0));
        nextBoard.resize(N, vector<bool>(N,0));

        srand48(time(0));

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
    void nextTurn(int threadID) {
        int chunkSize = N / maxThreads;
        int start = threadID * chunkSize;
        int end = (threadID + 1) * chunkSize;
        
        if ( N % maxThreads > threadID ) end++;

        for ( int i = start; i < end; i++) {
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

    }
    
    void nextTurn() {
        if ( maxThreads != 1 ) {
            //Initialize threads with responsibilities for each chunk
            for (int thread = 0; thread < maxThreads; thread++) {
                threadList[thread].set(thread, this);
                threadList[thread].start();
            }
            
            //Wait for each thread to complete before continuing
            for (int thread = 0; thread < maxThreads; thread++) {
                threadList[thread].kill();
            }
        }
        else {
            nextTurn(0);
        }
        
        
        endTurn();
    }

    void cleanup() { delete[] threadList; }

private:
    board currentBoard;
    board nextBoard;
    MyThread* threadList;
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

MyThread::MyThread() {};
    
void MyThread::set(int _argument, GameOfLife* gol ) {
    mArgument = _argument;
    life = gol;
}
    
void MyThread::start() {
    pthread_create(&mThreadId, 0,&MyThread::threadMethod, this);
}
    
void MyThread::doThings(GameOfLife* x, int threadNum) {
    x->nextTurn(threadNum);
}
    
void *MyThread::threadMethod(void *arg) {
    MyThread *_this=static_cast<MyThread *>(arg);
    _this->doThings(_this->getGame(), _this->getThread());
        
    return 0;
}

GameOfLife* MyThread::getGame() const {
        return life;
    }
    
int MyThread::getThread() const {
    return mArgument;
}
    
void MyThread::kill() {
    pthread_join(mThreadId, NULL);
}


int main( int numargs, char *args[]) {

    double starttime, endtime;
    int size, rounds, threadCount;


    size = atoi( args[1] );
    rounds = atoi( args[2] );
    threadCount = atoi( args[3] );

    GameOfLife life(size, threadCount);

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

    life.cleanup();
    endtime = gettime();
    

    ofstream output;
    output.open( "output.log", ios::out | ios::app );

    output << threadCount << " threads: " << size << " x " << size << " for " << rounds << " rounds took " << endtime - starttime << " seconds" << endl;

    output.close();

    cout << threadCount << " threads: " << size << " x " << size << " for " << rounds << " rounds took " << endtime - starttime << " seconds" << endl;
    
    return 0;
}
