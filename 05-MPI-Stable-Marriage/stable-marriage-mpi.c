#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Define tags for message passing
#define TAG_PROPOSE   1
#define TAG_ACCEPT    2
#define TAG_REJECT    3
#define TAG_DUMP      4
#define TAG_CHECKLIST     5
#define TAG_TERMINATE 6

// Create preference arrays (0 to n-1)
void init_preferences(int *pref, int n, int seed) {
    srand(seed);
    for (int i = 0; i < n; i++) pref[i] = i;

    // Fisher–Yates shuffle
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = pref[i];
        pref[i] = pref[j];
        pref[j] = tmp;
    }
}

// Creates a reverse lookup table for fast lookup
void build_rank_array(int *pref, int *rank_of_man, int n) {
    for (int i = 0; i < n; i++) {
        int man = pref[i];
        rank_of_man[man] = i; 
    }
}

// main man logic
void run_man(int rank, int n) {
    int pref[n];
    init_preferences(pref, n, rank + 123);

    int partner = -1;
    int next = 0;
    int done = 0;

    while (!done) {
        if (next >= n) {
            printf("Man %d is out of options. Error!", rank);
            fflush(stdout);
            break;
        }
        int woman_id;
        // If free
        if (partner == -1) {
            woman_id = pref[next++];
            int woman_rank = n + woman_id; // Women's MPI ranks start at 'n'

            printf("Trace: Man %d proposes to Woman %d\n", rank, woman_id);
            fflush(stdout);
            
            // Send empty payload; the TAG and SOURCE tell her everything she needs
            MPI_Send(NULL, 0, MPI_INT, woman_rank, TAG_PROPOSE, MPI_COMM_WORLD);
        }

        MPI_Status status;
        // Wait for any response
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // Calculate woman's ID from her rank
        if (status.MPI_TAG == TAG_ACCEPT) {
            partner = status.MPI_SOURCE - n;
            printf("Trace: Man %d is engaged to Woman %d\n", rank, woman_id);
            fflush(stdout);
        }
        else if (status.MPI_TAG == TAG_REJECT) {
            partner = -1; //for formality
            printf("Trace: Man %d is rejected by Woman %d\n", rank, woman_id);
            fflush(stdout);
        }
        else if (status.MPI_TAG == TAG_DUMP) {
            partner = -1;
            printf("Trace: Man %d is dumped by Woman %d\n", rank, woman_id);
            fflush(stdout);
        }
        else if (status.MPI_TAG == TAG_TERMINATE) done = 1;
    }

    printf("Result: Man %d final partner: Woman %d\n", rank, partner);
    fflush(stdout);
}

// main woman logic
void run_woman(int rank, int n) {
    int woman_id = rank - n; // Calculate this woman's 0-indexed ID

    int pref[n];
    int rank_of_man[n];
    init_preferences(pref, n, woman_id + 212);
    build_rank_array(pref, rank_of_man, n);

    int partner = -1;
    int done = 0;
    
    // The very first woman (rank n) conceptually starts with the CHECKLIST.
    bool has_checklist = (rank == n); 
    bool passed_checklist = false;

    while (!done) {
        MPI_Status status;
        // Listen for either a PROPOSAL, the CHECKLIST, or TERMINATE
        MPI_Recv(NULL, 0, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_PROPOSE) {
            int man = status.MPI_SOURCE; // Man's ID is exactly his rank (0 to n-1)

            if (partner == -1) {
                // First proposal: Must accept
                partner = man;
                MPI_Send(NULL, 0, MPI_INT, man, TAG_ACCEPT, MPI_COMM_WORLD);
            }
            else {
                // Compare new man vs current partner
                if (rank_of_man[man] < rank_of_man[partner]) {
                    int old = partner;
                    partner = man;
                    // Dump old, accept new
                    MPI_Send(NULL, 0, MPI_INT, old, TAG_DUMP, MPI_COMM_WORLD);
                    MPI_Send(NULL, 0, MPI_INT, man, TAG_ACCEPT, MPI_COMM_WORLD);
                }
                else {
                    // Reject new
                    MPI_Send(NULL, 0, MPI_INT, man, TAG_REJECT, MPI_COMM_WORLD);
                }
            }
        }
        else if (status.MPI_TAG == TAG_CHECKLIST) {
            has_checklist = true;
        }
        else if (status.MPI_TAG == TAG_TERMINATE) {
            done = 1;
        }

        // Checklist passing logic
        // If I am engaged, I have the checklist, and I haven't passed it yet...
        if (partner != -1 && has_checklist && !passed_checklist) {
            
            if (rank < (2 * n - 1)) {
                // Pass it to the next woman
                MPI_Send(NULL, 0, MPI_INT, rank + 1, TAG_CHECKLIST, MPI_COMM_WORLD);
            } 
            else {
                // I am the LAST woman. If I have the checklist and I'm engaged, 
                // it means ALL women are engaged. The algorithm is complete!
                printf("\n--- System Stable! Broadcasting Termination ---\n\n");
                fflush(stdout);
                
                // Broadcast terminate to everyone else
                for (int i = 0; i < 2 * n; i++) {
                    if (i != rank) {
                        MPI_Send(NULL, 0, MPI_INT, i, TAG_TERMINATE, MPI_COMM_WORLD);
                    }
                }
                done = 1; // Terminate myself
            }
            passed_checklist = true; // Ensure we only pass it once
        }
    }

    printf("Result: Woman %d final partner: Man %d\n", woman_id, partner);
    fflush(stdout);
}

// Main function
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    setvbuf(stdout, NULL, _IOLBF, 0);

    // We now strictly require exactly 2n processes
    if (size % 2 != 0) {
        if (rank == 0) printf("Error: Number of processes must be exactly even (2n).\n");
        fflush(stdout);
        MPI_Finalize();
        return 0;
    }

    int n = size / 2;

    // Ranks 0 to n-1 are Men. Ranks n to 2n-1 are Women.
    if (rank < n) {
        run_man(rank, n);
    } else {
        run_woman(rank, n);
    }

    MPI_Finalize();
    return 0;
}