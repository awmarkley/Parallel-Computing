/****************************************************************************** 
*  Sample C program for Homework1 in CS 432/632/732 - Spring 2016 Semester    *
*                                                                             *
*  Program to illustrate the use of static memory allocation to create a      *
*  2D-matrices and use gettime to measure wall clock time.                    *
*                                                                             *
*  To Compile: gcc -O hw1a.c (to print matrices add -DDEBUG_PRINT)            *
*  To run: ./a.out <size>                                                     *
*                                                                             *
*  Author: Purushotham Bangalore                                              *
*  Email: puri@uab.edu                                                        *
*  Date: January 9, 2016                                                      *
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* 
   Edit the values below to change the matrix size OR 
   Delete the line below and compile with -D flag, e.g., gcc -O -DN=10 hw1a.c 
*/
#define N 1000

double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void initarray(double a[N][N], double value) {
  int i,j;

  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
      // a[i][j] = drand48()*value;
      a[i][j] = value;
}

void printarray(double a[N][N]) {
  int i,j;
  
  for (i=0; i<N; i++) {
    for (j=0; j<N; j++)
      printf("%f ", a[i][j]);
    printf("\n");
  }
}

void matmul(double a[N][N], double b[N][N], double c[N][N]) 
{
    int i, j, k;
    for (i=0; i<N; i++)
      for (j=0; j<N; j++) 
	for (k=0; k<N; k++)
	  c[i][j] += a[i][k]*b[k][j];
}

int main(int argc, char **argv) 
{
    int i, j, k;
    /* Why are these arrays declared static?  */
    /* What happens if static keyword is removed? */
    static double a[N][N], b[N][N], c[N][N]; 
    double starttime, endtime;               

    /* Initialize the matrices */
    srand48(123456);
    initarray(a, (double)(1.0));
    initarray(b, (double)(2.0));
    initarray(c, (double)0.0);

    /* Perform matrix multiplication */
    starttime = gettime();
    matmul(a,b,c);
    endtime = gettime();

#ifdef DEBUG_PRINT
    printarray(a);
    printf("\n");
    printarray(b);
    printf("\n");
    printarray(c);
#endif

    printf("Time taken = %lf seconds\n", endtime-starttime);

    return 0;
}
