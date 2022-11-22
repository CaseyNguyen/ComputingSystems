// Casey Nguyen
// 05/04/2021
// social_distancing.cpp
//
// PURPOSE: This program uses multi-threading and semaphores to process a
// user-inputted number of vaccinated and unvaccinated children in their
// attempts to play on the playground. If there is an unvaccinated child in the
// playground, they have sole access of the playground while any number of
// vaccinated children can play so as long as an unvaccinated child isn't
// among them.
//
// INPUT: argc, argv[]
// argc - The number of arguments provided in the command line.
// argv[] - Contents of the arguments provided in the command line.
//
// PROCESSING:
// argc - Checks if argc == 3.
// argv[] - Checks if argv[1] and argv[2] are integers.
//
// OUTPUT:
// argc - Exits the program if it is not equal to 3, else proceeds.
// argv[] - Exits the program if the provided values aren't int, else proceeds.
//
// EXAMPLE (Note: Program will run forever until manually stopped.):
/*
  ./social_distancing 3 3
  V Thread 1 acquired mutex for incrementing the count.
  V Thread 1 trying to acquire play_mutex.
  V Thread 1 about to release mutex after incrementing.
  NV Thread 4 trying to acquire play_mutex.
  V Thread 1 playing!
  V Thread 2 acquired mutex for incrementing the count.
  V Thread 2 about to release mutex after incrementing.
  V Thread 2 playing!
  V Thread 3 acquired mutex for incrementing the count.
  V Thread 3 about to release mutex after incrementing.
  V Thread 3 playing!
  NV Thread 5 trying to acquire play_mutex.
  NV Thread 6 trying to acquire play_mutex.
  V Thread 1 playing done!
 */

#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
using namespace std;

int vaccinated_kids_count = 0; // number of vaccinated kids at structure
sem_t mutex; // sets mutual exclusion during vaccinated_kids_count change
sem_t play_mutex; // sets mutual exclusion for the structure

void checkArgs(int, char*, char*);
// PURPOSE: Checks if the proper number of values were inputted in command line.
// IN: numArg - The number of arguments that the user inputted.
//     argv[1] and argv[2] - Character strings to be checked if they are int.
// OUT: Exits the program on failure, else it continues.

void Play(int, bool);
// PURPOSE: Counts as a child "playing".
// IN: idVal - The current threadId "playing".
//     vaccinated - Checks if this child is vaccinated for output purposes.
// OUT: Shows the child's vaccination status and id, counts as "play".

void* RunNV(void*);
// PURPOSE: Processes the thread of an unvaccinated kid.
// IN: nv - Represents the threadId of some unvaccinated kid.
// OUT: Ends the thread.

void* RunV(void*);
// PURPOSE: Processes the thread of a vaccinated kid.
// IN: Represents the threadId of some vaccinated kid.
// OUT: Ends the thread.

int main(int argc, char* argv[]){
  checkArgs(argc, argv[1], argv[2]);
  sem_init(&mutex, 0, 1);
  sem_init(&play_mutex, 0, 1);
  int numVac = atoi(argv[1]); // number of vaccinated kids
  int sumKids = atoi(argv[1]) + atoi(argv[2]); // total number of kids
  int threadId[sumKids]; // holds the id's
  bool vaccineStatus[sumKids]; // dynamic array to match threadId
  for (int i = 0; i < sumKids; i++){
    threadId[i] = (i + 1);
    if (i < numVac) // assign these as "vaccinated"
      vaccineStatus[i] = true;
    else // assign these as "unvaccinated"
      vaccineStatus[i] = false;
  }
  pthread_t thread[sumKids]; // threads to run operations with kids
  for (int i = 0; i < sumKids; i++){
    if (vaccineStatus[i])
      pthread_create(&thread[i], NULL, RunV, &threadId[i]);
    else
      pthread_create(&thread[i], NULL, RunNV, &threadId[i]);
  }
  for (int i = 0; i < sumKids; i++)
    pthread_join(thread[i], NULL);
  sem_destroy(&mutex);
  sem_destroy(&play_mutex);
  return 0;
}

// Methods
void checkArgs(int numArg, char* arg1, char* arg2){
  // Breakdown: If we have 3 arguments and arg1 and arg2 aren't floats, proceed.
  if ((numArg != 3) || (atof(arg1) != atoi(arg1)) || (atof(arg2) != (atoi(arg2)))){
    printf("Invalid arguments provided. Exiting...\n");
    exit(1);
  }
}

void Play(int idVal, bool vaccinated){
  printf("%s Thread %d playing!\n", (vaccinated ? "V" : "NV"), idVal);
  sleep(1);
  printf("%s Thread %d playing done!\n", (vaccinated ? "V" : "NV"), idVal);
}

void* RunNV(void* nv){
  int idNum; // threadId
  idNum = *(int*)nv;
  do{
    printf("NV Thread %d trying to acquire play_mutex.\n", idNum);
    sem_wait(&play_mutex);
    Play(idNum, false);
    sem_post(&play_mutex);
  }while(true);
  pthread_exit(NULL);
}

void* RunV(void* v){
  int idNum; // threadId
  idNum = *(int*)v;
  do{
    sem_wait(&mutex);
    printf("V Thread %d acquired mutex for incrementing the count.\n", idNum);
    vaccinated_kids_count++;
    if(vaccinated_kids_count == 1){
      printf("V Thread %d trying to acquire play_mutex.\n", idNum);
      sem_wait(&play_mutex);
    }
    printf("V Thread %d about to release mutex after incrementing.\n", idNum);
    sem_post(&mutex);
    Play(idNum, true);
    sem_wait(&mutex);
    vaccinated_kids_count--;
    if(vaccinated_kids_count == 0){
      printf("V Thread %d about to release play_mutex.\n", idNum);
      sem_post(&play_mutex);
    }
    printf("V Thread %d about to release mutex after decrementing.\n", idNum);
    sem_post(&mutex);
  }while(true);
  pthread_exit(NULL);
}





