// 8-Queens problem using OpenMP tasks
// Skeleton framework for homework solution

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>
#define RUNS 100
#define REPEAT 100

// Global counter for number of valid solutions
// Must be updated in a thread-safe way
int solution_count = 0;
double times[RUNS]; // to find the median

// Check whether placing a queen at (row, col) is safe
// q_placed_row[i] = row index of queen in column i
int is_safe(int q_placed_row[], int col, int row) {
    for (int i = 0; i < col; i++) {
        // Row check
        if (q_placed_row[i] == row) return 0;
        // Diagonal check
        if (abs(i - col) == abs(q_placed_row[i] - row)) return 0;
    }
    return 1;
}


// Recursive function to place queens column by column
void solve_queens(int q_placed_row[], int col) {
    // Base case: all columns filled → found a solution
    if (col == 8) {
        #pragma omp atomic
        solution_count++;
        return;
    }

    // Try all rows in the current column
    for (int row = 0; row < 8; row++) {
        // Check if position is valid
        if (is_safe(q_placed_row, col, row)) {

            // Create a local copy of the board
            int new_q_placed_row[8];
            for (int i = 0; i < col; i++)
                new_q_placed_row[i] = q_placed_row[i];

            new_q_placed_row[col] = row;

            // Create tasks only for upper levels of recursion
            // Too many small tasks cause overhead, so we limit parallelism depth
            if (col < 2) {
                #pragma omp task firstprivate(new_q_placed_row, col)
                // This recursive call becomes a separate task
                solve_queens(new_q_placed_row, col + 1);
            } else {
                // Deeper recursion done sequentially to reduce overhead
                solve_queens(new_q_placed_row, col + 1);
            }
        }
    }

    // Wait for all child tasks to complete before returning
    #pragma omp taskwait
}

// Utility function for the sorting, to find the median
int compare_doubles(const void *a, const void *b) {
    double x = *(double *)a;
    double y = *(double *)b;

    if (x < y) return -1;
    if (x > y) return 1;
    return 0;
}


int main(void) {
    int q_placed_row[8];
    double start_time, end_time;
    double means[RUNS];
    double sequential_time;

    for (int thrd = 1; thrd <= 8; thrd++) {

        omp_set_num_threads(thrd);

        for (int r = 0; r < RUNS; r++) {
            start_time = omp_get_wtime();
            #pragma omp parallel
            {
                #pragma omp single
                {
                    //start_time = omp_get_wtime();

                    for (int i = 0; i < REPEAT; i++) {
                        solution_count = 0;
                        solve_queens(q_placed_row, 0);
                    }

                    //end_time = omp_get_wtime();
                }
            }
            end_time = omp_get_wtime();
            means[r] = (end_time - start_time) / REPEAT;
        }

        qsort(means, RUNS, sizeof(double), compare_doubles);
        double median_mean = means[RUNS / 2];

        if (thrd == 1) sequential_time = median_mean;

        printf("Threads: %d\n", thrd);
        printf("Solutions: %d\n", solution_count);
        printf("Median of mean execution time: %f seconds\n", median_mean);
        printf("Speedup: %f\n", sequential_time/median_mean);
        printf("\n");
    }


    return 0;
}
