/*Part a: Parallel using barrier adapted from given matrixSum.c 

ID1217 Group 68
*/

#ifndef _REENTRANT 
#define _REENTRANT 
#endif 
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <limits.h>
#define MAXSIZE 10000  /* maximum matrix size */
#define MAXWORKERS 10   /* maximum number of workers */

pthread_mutex_t barrier;  /* mutex lock for the barrier */
pthread_cond_t go;        /* condition variable for leaving */
int numWorkers;           /* number of workers */ 
int numArrived = 0;       /* number who have arrived */

/* a reusable counter barrier */
void Barrier() {
    pthread_mutex_lock(&barrier);
    numArrived++;
    if (numArrived == numWorkers) {
        numArrived = 0;
        pthread_cond_broadcast(&go);
    } else
        pthread_cond_wait(&go, &barrier);
    pthread_mutex_unlock(&barrier);
}

/* timer */
double read_timer() {
    static bool initialized = false;
    static struct timeval start;
    struct timeval end;
    if( !initialized )
    {
        gettimeofday( &start, NULL );
        initialized = true;
    }
    gettimeofday( &end, NULL );
    return (end.tv_sec - start.tv_sec) + 1.0e-6 * (end.tv_usec - start.tv_usec);
}

/*arrays to store partial results (MY CHANGES)*/
long long sums[MAXWORKERS]; //long long to avoid overflow (64 bit)
int min_vals[MAXWORKERS]; 
int max_vals[MAXWORKERS];
int min_row_index[MAXWORKERS], min_col_index[MAXWORKERS];
int max_row_index[MAXWORKERS], max_col_index[MAXWORKERS];

double start_time, end_time; /* start and end times */
int size, stripSize;  /* assume size is multiple of numWorkers */
int matrix[MAXSIZE][MAXSIZE]; /* matrix */

void *Worker(void *);

/* read command line, initialize, and create threads */
int main(int argc, char *argv[]) {
    int i, j;
    long l; /* use long in case of a 64-bit system */
    pthread_attr_t attr;
    pthread_t workerid[MAXWORKERS];

    /* set global thread attributes */
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    /* initialize mutex and condition variable */
    pthread_mutex_init(&barrier, NULL);
    pthread_cond_init(&go, NULL);

    /* read command line args if any */
    size = (argc > 1)? atoi(argv[1]) : MAXSIZE;
    numWorkers = (argc > 2)? atoi(argv[2]) : MAXWORKERS;
    if (size > MAXSIZE) size = MAXSIZE;
    if (numWorkers > MAXWORKERS) numWorkers = MAXWORKERS;
    stripSize = size/numWorkers;

    /* initialize the matrix */
    for (i = 0; i < size; i++) {
	    for (j = 0; j < size; j++) {
            matrix[i][j] = rand()%99; //1; 
	    }
    }

  /* print the matrix */
#ifdef DEBUG
    for (i = 0; i < size; i++) {
	    printf("[ ");
	    for (j = 0; j < size; j++) {
	        printf(" %d", matrix[i][j]);
	    }
	    printf(" ]\n");
    }
#endif

    /* do the parallel work: create the workers */
    start_time = read_timer();
    for (l = 0; l < numWorkers; l++)
        pthread_create(&workerid[l], &attr, Worker, (void *) l);
    pthread_exit(NULL);
}

/* Each worker sums the values in one strip of the matrix.
   After a barrier, worker(0) computes and prints the total */
/*Also finds min and max values and their indices (MY CHANGES)*/
void *Worker(void *arg) {
    long myid = (long) arg;
    long long total = 0; //changed to long long to avoid overflow
    int min_val = INT_MAX; //guarantee first value checked is smaller
    int max_val = INT_MIN; //guarantee first value checked is larger
    int min_row = -1, min_col = -1;
    int max_row = -1, max_col = -1;
    int i, j, first, last;

#ifdef DEBUG
    printf("worker %d (pthread id %d) has started\n", myid, pthread_self());
#endif

    /* determine first and last rows of my strip */
    first = myid*stripSize;
    last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

  /*min, max, sum values in my strip (MY CHANGES)*/
    total = 0;
    for (i = first; i <= last; i++) {
        for (j = 0; j < size; j++) {
            int value = matrix[i][j];
            total += value;
            if (value < min_val) {
                min_val = value;
                min_row = i;
                min_col = j;
            }
            if (value > max_val) {
                max_val = value;
                max_row = i;
                max_col = j;
            }
        }
    }

    /*store partial results (MY CHANGES)*/
    sums[myid] = total;
    min_vals[myid] = min_val;
    max_vals[myid] = max_val;
    min_row_index[myid] = min_row;
    min_col_index[myid] = min_col;
    max_row_index[myid] = max_row;
    max_col_index[myid] = max_col;

    Barrier(); //all threads call barrier to sync

    /*after barrier only worker 0 does computations and prints (MY CHANGES)*/
    if (myid == 0) {
        long long final_sum = 0;
        int final_min = INT_MAX;
        int final_max = INT_MIN;
        int final_min_row, final_min_col, final_max_row, final_max_col;

        for (i = 0; i < numWorkers; i++) {
            final_sum += sums[i];

            if (min_vals[i] < final_min) {
                final_min = min_vals[i];
                final_min_row = min_row_index[i];
                final_min_col = min_col_index[i];
            }

            if (max_vals[i] > final_max) {
                final_max = max_vals[i];
                final_max_row = max_row_index[i];
                final_max_col = max_col_index[i];
            }
        }

        /* get end time */
        end_time = read_timer();

        /* print results */
        printf("The total is %lld\n", final_sum);
        printf("The minimum value is %d at position (%d, %d)\n", final_min, final_min_row, final_min_col);
        printf("The maximum value is %d at position (%d, %d)\n", final_max, final_max_row, final_max_col);
        printf("The execution time is %g sec\n", end_time - start_time);
    }
    return NULL;
}
