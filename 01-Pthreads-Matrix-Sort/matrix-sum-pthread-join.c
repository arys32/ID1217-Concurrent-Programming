/*Part b: main prints results, no barrier and no partial arrays

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

int numWorkers;           /* number of workers */ 

/*struct named ThreadResult to return data from threads*/
typedef struct {
    long long total;
    int max_val;
    int min_val;
    int max_row, max_col;
    int min_row, min_col;
} ThreadResult;

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
    //pthread_exit(NULL); //main doesnt retire
    long long final_sum = 0;
    int final_max = INT_MIN;
    int final_min = INT_MAX;
    int final_max_row, final_max_col, final_min_row, final_min_col;
  
    for (l = 0; l < numWorkers; l++) {
        ThreadResult *result;
        pthread_join(workerid[l], (void**) &result);
      
        final_sum += result->total;
      
        if (result->max_val > final_max) {
            final_max = result->max_val;
            final_max_row = result->max_row;
            final_max_col = result->max_col;
        }
        if (result->min_val < final_min) {
            final_min = result->min_val;
            final_min_row = result->min_row;
            final_min_col = result->min_col;
        }
      free(result); //deallocate memory of worker's result
    }
    end_time = read_timer();

    printf("The total is %lld\n", final_sum);
    printf("The minimum value is %d at position (%d, %d)\n", final_min, final_min_row, final_min_col);
    printf("The maximum value is %d at position (%d, %d)\n", final_max, final_max_row, final_max_col);
    printf("The execution time is %g sec\n", end_time - start_time);
  
    pthread_exit(NULL);
}


/*Each worker finds sum, min and max values and their indices without a barrier function*/
void *Worker(void *arg) {
    long myid = (long) arg;
    int i, j, first, last;
  
    ThreadResult *result = (ThreadResult *) malloc(sizeof(ThreadResult));
    result->total = 0;
    result->min_val = INT_MAX;
    result->max_val = INT_MIN;

    first = myid * stripSize;
    last = (myid == numWorkers - 1) ? (size - 1) : (first + stripSize - 1);

    for (i = first; i <= last; i++) {
        for (j = 0; j < size; j++) {
            int val = matrix[i][j];
            result->total += val;
            if (val < result->min_val) {
                result->min_val = val;
                result->min_row = i;
                result->min_col = j;
            }
            if (val > result->max_val) {
                result->max_val = val;
                result->max_row = i;
                result->max_col = j;
            }
        }
    }
    pthread_exit((void*) result);
}
