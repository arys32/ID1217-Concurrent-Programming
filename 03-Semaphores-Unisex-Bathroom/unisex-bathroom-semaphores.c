#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

//semaphores
sem_t gender_key;
sem_t gate;
sem_t men_count_lock;    //protects men_inside counter
sem_t women_count_lock;  //protects women_inside counter

//counters
int men_inside_count= 0;
int women_inside_count = 0;

void* man_thread(void* arg) {
    int id = *(int*)arg;
    srand(time(NULL)^id);
    while (1) {
        //outside bathroom
        sleep(rand() % 5 + 1); 

        printf("Man %d wants to use the bathroom.\n", id);

        //man enters bathroom
        sem_wait(&gate);
        sem_wait(&men_count_lock);
        men_inside_count++;
        if (men_inside_count == 1) { 
            sem_wait(&gender_key); //first man takes the gender specific key to lock out women
        }
        printf(">>> Man %d entered. (Men inside: %d)\n", id, men_inside_count); //moved print inside lock
        sem_post(&men_count_lock); 
        sem_post(&gate);

        //inside bathroom
        sleep(rand() % 3 + 1);

        //exits
        sem_wait(&men_count_lock);
        men_inside_count--;
        printf("<<< Man %d leaving. (Men inside: %d)\n", id, men_inside_count);
        if (men_inside_count == 0) {
            printf("--- Bathroom is EMPTY (Men unlocked it) ---\n"); 
            sem_post(&gender_key); //if last men exits, unlock the gender key for any gender to enter
        }
        sem_post(&men_count_lock);
    }
    return NULL;
}

void* woman_thread(void* arg) {
    int id = *(int*)arg;
    srand(time(NULL)^id);
    while (1) { //infinite loop to cycle the states
        //outside bathroom
        sleep(rand() % 5 + 1); 

        printf("Woman %d wants to use the bathroom.\n", id);

        //woman enters
        sem_wait(&gate); 
        sem_wait(&women_count_lock);
        
        women_inside_count++;
        if (women_inside_count == 1) { 
            sem_wait(&gender_key); //if first woman, take key to lock out men
        }
        printf(">>> Woman %d entered. (Women inside: %d)\n", id, women_inside_count); //moved print inside lock
        sem_post(&women_count_lock);
        sem_post(&gate); //gate is free to use for next person

        //inside bathroom
        sleep(rand() % 3 + 1); 

        //exit bathroom
        sem_wait(&women_count_lock);
        women_inside_count--;
        printf("<<< Woman %d leaving. (Women inside: %d)\n", id, women_inside_count);
        if (women_inside_count == 0) {
            printf("--- Bathroom is EMPTY (Women unlocked it) ---\n"); 
            sem_post(&gender_key); //last woman releases key for other gender
        }
        sem_post(&women_count_lock);
    }
    return NULL;
}

int main() {
    int num_men = 5;
    int num_women = 5;
    pthread_t men[num_men], women[num_women];
    int ids[10]; //to store ids for threads

    //initialize semaphores
    sem_init(&gender_key, 0, 1); 
    sem_init(&gate, 0, 1);
    sem_init(&men_count_lock, 0, 1);
    sem_init(&women_count_lock, 0, 1);

    //create the threads
    for (int i = 0; i < num_men; i++) {
        ids[i] = i + 1;
        pthread_create(&men[i], NULL, man_thread, &ids[i]);
    }
    for (int i = 0; i < num_women; i++) {
        ids[i + num_men] = i + 1;
        pthread_create(&women[i], NULL, woman_thread, &ids[i + num_men]);
    }

    //join to make sure main doesnt retire
    for (int i = 0; i < num_men; i++) pthread_join(men[i], NULL);
    for (int i = 0; i < num_women; i++) pthread_join(women[i], NULL);

    return 0;
}