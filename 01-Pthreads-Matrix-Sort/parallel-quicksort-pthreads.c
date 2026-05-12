#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#define MAX_THREADS 8 // To limit recursion depth
#define MIN_SIZE 1000 // Subarrays smaller than this are sorted sequentially to reduce thread overhead

typedef struct { // thread_func() allows a single argument, so we pass multiple values with a struct
    int *array;
    int low;
    int high;
    int depth;
} quick_args;

// Synchronization mutexes
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
int active_threads = 0;

// Partitions the array into two parts (Lomuto partition)
int partition(int *arr, int low, int high) {
    int pivot = arr[high];  // The last element of the subarray is selected as the pivot point
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    int tmp = arr[i+1];
    arr[i+1] = arr[high];
    arr[high] = tmp;
    return i + 1;
}

// Sequential quicksort function used for small subarrays
void quicksort_seq(int *arr, int low, int high) {
    if (low < high) {
        int p = partition(arr, low, high);
        quicksort_seq(arr, low, p - 1);
        quicksort_seq(arr, p + 1, high);
    }
}

// Main parallel quicksort function
void *quicksort_parallel(void *args) { 
    quick_args *a = (quick_args *)args;

    if (a->low < a->high) {
        // Use sequential quicksort for small arrays
        if ((a->high - a->low) < MIN_SIZE) quicksort_seq(a->array, a->low, a->high);
        else {
            int p = partition(a->array, a->low, a->high);
            quick_args left  = {a->array, a->low, p - 1, a->depth + 1};
            quick_args right = {a->array, p + 1, a->high, a->depth + 1};
            pthread_t t;

            if (a->depth < MAX_THREADS) {
                pthread_mutex_lock(&count_mutex); 
                active_threads++;
                pthread_mutex_unlock(&count_mutex);

                pthread_create(&t, NULL, quicksort_parallel, &left);
                quicksort_parallel(&right);
                pthread_join(t, NULL);
            } else {
                quicksort_parallel(&left);
                quicksort_parallel(&right);
            }
        }
    }

    // Thread completion
    pthread_mutex_lock(&count_mutex);
    active_threads--;
    if (active_threads == 0) pthread_cond_signal(&done_cond);
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

// Print array testing funtion 
void print_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect number of arguments");
        return 1;
    }

    int n = atoi(argv[1]);
    int *arr = malloc(n * sizeof(int));

    for (int i = 0; i < n; i++) arr[i] = rand(); // Fills array with random values
    
    //print_array(arr, n);
    
    struct timeval start, end;

    pthread_mutex_lock(&count_mutex);
    active_threads = 1;   // main worker thread
    pthread_mutex_unlock(&count_mutex);

    gettimeofday(&start, NULL);

    quick_args args = {arr, 0, n - 1, 0};
    pthread_t main_thread;

    pthread_create(&main_thread, NULL, quicksort_parallel, &args);

    // Wait until all threads are done
    pthread_mutex_lock(&count_mutex);
    while (active_threads > 0) {
        pthread_cond_wait(&done_cond, &count_mutex);
    }
    pthread_mutex_unlock(&count_mutex);

    pthread_join(main_thread, NULL);

    gettimeofday(&end, NULL);

    double time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Execution time: %.6f seconds\n", time);

    //print_array(arr, n);

    free(arr);
    return 0;
}
