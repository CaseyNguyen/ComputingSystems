// Casey Nguyen
// 05/03/2021
// vaccines.cpp
//
// PURPOSE: This program receives a hard-coded set of 21 files and uses
// multi-threading to process the tally of strings identified as "v1", "v2", and
// "v3" in the file. The file number is equivalent to the index of each
// individual thread incremented by 1. This operation is performed by first
// creating the threads, reading the files in the start process, and joining
// the threads together to get the final results.
//
// INPUT: (N/A)
//
// EXAMPLE (Note: As there is no input, this should be the result):
/*
  Total Vaccine Count:
  v1 = 47
  v2 = 47
  v3 = 25
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <pthread.h>
using namespace std;

const int VAC_THREADS = 21; // hard-coded number of threads to be created

struct VaccineData{
  int threadId; // id value of an individual thread
  string readFile; // file that the thread will read from
  int tally1; // tallies "v1" found in a city[x].txt file
  int tally2; // ditto, for "v2"
  int tally3; // ditto, for "v3"
};

// Methods:
string getFile(int);
// PURPOSE: Returns a file name based on the threadId.
// IN: index - Integer value of the current threadId.
// OUT: File name, which is essentially a string appended with city[index].txt.

void* runThread(void*);
// PURPOSE: Start process for our 21 threads.
// IN: inThread - Pointer to the data of the current thread.
// OUT: Returns a struct to the main function holding the tallies of a thread.

void checkError(int);
// PURPOSE: Checks if an error with the thread has occurred.
// IN: check - The value that the pthread_create/join function returns.
// OUT: Simply runs checks, but will abort the program if necessary (exit(-1)).

void showTally(int, int, int);
// PURPOSE: Shows the final tally of all vaccines.
// IN: finalTally - The accumulated data from all threads.
// OUT: Prints the total number of vaccines found in all threads.

int main(){
  pthread_t thread[VAC_THREADS]; // variable that initializes 21 threads
  pthread_attr_t threadAttr; // variable to let our threads join (to be set)
  pthread_attr_init(&threadAttr);
  pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_JOINABLE);
  VaccineData tallyData, threadData[VAC_THREADS];
  // tallyData - used to count the data from our threads after join operations
  // threadData - holds the data of these 21 threads
  int tot1 = 0, tot2 = 0, tot3 = 0; // tallies of individual vaccines
  VaccineData *addData; // struct that will hold the data of each vaccine count

  // This loop is where all 21 threads reside in.
  for (int i = 0; i < VAC_THREADS; i++){
    // First, let's initialize the values of our data.
    threadData[i].threadId = i;
    threadData[i].readFile = getFile(i);
    threadData[i].tally1 = 0;
    threadData[i].tally2 = 0;
    threadData[i].tally3 = 0;

    // Now, we can create a thread:
    int s; // used so we can check if something goes wrong with the thread (1)
    s = pthread_create(&thread[i], &threadAttr, runThread, (void*)&threadData[i]);
    checkError(s);

    // And we can join them:
    s = pthread_join(thread[i], (void**)&addData);
    checkError(s);
    tot1 += addData->tally1;
    tot2 += addData->tally2;
    tot3 += addData->tally3;
  }
  // Finally, let's show the tally of all of the vaccine counts.
  showTally(tot1, tot2, tot3);
  return 0;
}

// Definitions:
string getFile(int index){
  string fileName = "/home/fac/mishraa/cpsc3500/pa3a/city";
  // name of the file that will be returned
  string cityNum = to_string(index+1); // converst the city # to a string...
  fileName += (cityNum + ".txt"); // ...then appends it to fileName
  return fileName;
}

void* runThread(void* inThread){
  VaccineData *currThread; // allows us to perform operations with the thread
  currThread = (VaccineData*) inThread;
  ifstream inFile(currThread->readFile); // allows us to read the file
  if(inFile.is_open()){
    string readData; // reads a line from inFile
    while(getline(inFile, readData)){
      if (readData == "v1")
        (currThread->tally1)++;
      else if (readData == "v2")
        (currThread->tally2)++;
      else
        (currThread->tally3)++;
    }
  }
  else{
    cout << "Error: Failed to open file. Exiting..." << endl;
    exit(1);
  }
  inFile.close();
  return (void*) currThread;
}

void checkError(int check){
  if (check == 1){
    cout << "Error: Something went wrong with a thread. Aborting..." << endl;
    exit(-1);
  }
}

void showTally(int finalOne, int finalTwo, int finalThree){
  cout << "Total Vaccine Count: " << endl;
  cout << "v1 = " << finalOne << endl;
  cout << "v2 = " << finalTwo << endl;
  cout << "v3 = " << finalThree << endl;
}



