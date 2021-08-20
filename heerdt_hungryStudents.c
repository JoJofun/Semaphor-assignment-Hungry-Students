//nicholas heerdt
//cs 370
//project3
//heerdt_hungryStudents.c
//compile as "gcc -Wall -pedantic -pthread -o heerdt_hungryStudents heerdt_hungryStudents.c"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <stdbool.h>

sem_t studentMtx;
sem_t empty_pot;
sem_t full_pot;
sem_t active;//this is used to keep track of the number of active threads so it can close cleanly

unsigned int const REFILL_SIZE = 5;
unsigned int const MAX_REFILLS = 5;
unsigned int currPotServing = 0;
unsigned int currPotRefils = 0;

void putServingsInPot();
void studentEats(int stuNum);
void getServingFromPot();

void* RA();
void* hungryStudents(void* args);

int main( int argc, char *argv[] )  {

    if (argc != 2) {
        printf("Error invalid number of command line args\n");
    }
    else {

        char* inputstr = argv[1];
        unsigned int totalNum = atoi(inputstr);

        if( argc == 2 && totalNum <= 20 && totalNum >= 3) {
            pthread_t RATread;
            pthread_t* threadPtr;
            int* t_id = NULL;

            threadPtr = (pthread_t*) malloc(sizeof(pthread_t)*totalNum);
            t_id = (int*) malloc(sizeof(int)*totalNum);

            //initialize semaphores
            sem_init(&studentMtx, 1, 1);
            sem_init(&full_pot, 1, 0);
            sem_init(&empty_pot, 1, 0);
            sem_init(&active, 1, totalNum+1);


            //make threads
            pthread_create(&RATread, NULL, &RA, NULL);
            for (int i = 0; i < totalNum; i++) {
                t_id[i] = i;
                pthread_create(&threadPtr[i], NULL, &hungryStudents, &t_id[i]);
            }

            //join threads
            pthread_join(RATread, NULL);
            for (int i = 0; i < totalNum; i++) {
                sem_wait(&active);
                pthread_join(threadPtr[i], NULL);
            }

            //wait for threads to finish
            for (int i = 0; i < totalNum + 1; i++) {
                sem_wait(&active);
            }
            printf("\nGame Over, thank you for playing\n");
            free(threadPtr);
            free(t_id);
        }
        else {
            printf("invalid input please use a number from 3 to 20\n");
        }
    }

    return 0;
}

void* hungryStudents(void* args) {
    int* studentID = (int*) args;
    int toterm = 0;
    //int studentID = (*(int*)&args[0]);
    //printf("Student id: %d\n", studentID);
    while(toterm == 0) {

        //~ P(studentMTX)
        sem_wait(&studentMtx);

        if (currPotServing == 0 && currPotRefils == MAX_REFILLS) {
            toterm = 1;
            sem_post(&studentMtx);
        }
        else {
            if (currPotServing == 0) {
                //~ v(empty)
                sem_post(&empty_pot);
                //~ p(full)
                sem_wait(&full_pot);
            }
            getServingFromPot();
            sem_post(&studentMtx);
            //~ studentEats
            studentEats(*studentID);
        }


    }
    sem_post(&active);
    //printf("END OF STUDENT:%d\n", *studentID);
    return NULL;
}

void* RA() {
    int toterm = 0;
    //putServingsInPot();
    while(toterm == 0) {


        sem_wait(&empty_pot);
        //P(empty pot)
        if (currPotRefils < MAX_REFILLS) {
            putServingsInPot();
            //V full pot
            if (currPotRefils == MAX_REFILLS) {
                printf("\033[0;31mHey, this is the last of the ramen noodles...\033[0m\n");
                toterm = 1;
            }
            sem_post(&full_pot);

        }
    }
    sem_post(&active);
    //printf("END OF RA\n");
    return NULL;
}

void putServingsInPot() {
    currPotServing = REFILL_SIZE;
    currPotRefils++;
    printf("\033[0;31mOK, fine here are some more Ramen noodles (%d)\033[0m\n", REFILL_SIZE);
    usleep(rand() % 500000); //This is 100 for testing purposes needs to be changed
}

void studentEats(int stuNum) {
    printf("\033[0;32mStudent %d eating, yum...\033[0m\n", stuNum);
    usleep(rand() % 1000000);//This is 100 for testing purposes needs to be changed
}

void getServingFromPot() {
    //~ decrements current pot servings
    currPotServing--;
    //~ wait a random amount of time to get a serving
    usleep(rand() % 1000);
}
