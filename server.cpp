// Casey Nguyen
// server.cpp
// 05/31/2021
//
// PURPOSE: Runs the server for the "Are You Pyschic!" (not trademarked) game.
// The first half of the main function essentially boots up the server.
// The second half of the main function runs this server with pthreads, with
// each individual client counting as a thread. The server will send if the
// guess is too high or low until the correct answer is provided, then send
// a leaderboard of the current standings of players. A few messages are shown
// to demonstrate the server is working properly.
//
// INPUT: portno - The port number of the server, anything of 1024 and beyond.
//
// OUTPUT: Sets the port number for where other clients to play the game.
//
// EXAMPLE:
/* ./server 1024
   Server is online!
   Randomly-generated number: 610.
   Username: Casey
   Closing a connection on player victory.
   Randomly-generated number: 268.
*/
// It is worth noting that 'n' often represents the number of characters read.
// Another common variable is buffer[]; this is the feed of data being read.
//
// Lots and lots of libraries to include...
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
using namespace std;

const int BUFFER_SIZE = 100; // The size of the buffer where data is sent.

// The following is the pseduo-leaderboard.
long leader1 = 0, leader2 = 0, leader3 = 0; // These are the best turns.
string name1 = "", name2 = "", name3 = ""; // These are the best players.
sem_t boardMutex; // Ensures mutual exclusion upon updating the leaderboard.

// These are the arguments passed into each thread.
struct ClientArgs{
  int clientSock; // The socket provided by the client.
  int correctNum; // The correct number that a user needs to get.
};

// These functions are defined in the README.txt file.
void error(char*);
void checkArgs(int);
void checkSocket(int);
void checkStatus(int);
void checkRead(int, int);
void checkWrite(int);
void updateBoard(string, int);
string readName(int);
int readNum(int);
void sendNum(long, int);
void sendName(string, string, int, int);
void sendBoard(int);
void *runClient(void*);

int main(int argc, char *argv[]){
  checkArgs(argc);
  srand(time(NULL));
  sem_init(&boardMutex, 0, 1);
  int sockfd, newsockfd, portno, n; // Sock file descriptor, in sock, port num, n.
  socklen_t clilen;
  char buffer[BUFFER_SIZE];
  struct sockaddr_in serv_addr, cli_addr;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  checkSocket(sockfd);
  bzero((char *) &serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);
  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
    char msg[] = "ERROR: Issue on binding.\n";
    error(msg);
  }
  listen(sockfd, 5);
  clilen = sizeof(cli_addr);
  printf("Server is online!\n");

  // Handling of a client occurs in this loop:
  while(true){
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    newsockfd = accept(sockfd, (struct sockaddr *) &clientAddr, &addrLen);
    checkSocket(newsockfd);
    ClientArgs *clientArgs = new ClientArgs;
    clientArgs->clientSock = newsockfd;
    clientArgs->correctNum = rand () % 1000;
    printf("Randomly-generated number: %d.\n", clientArgs->correctNum);

    // Now, we create a new thread and run the game.
    pthread_t threadId;
    int status = pthread_create(&threadId, NULL, runClient, (void *) clientArgs);
    checkStatus(status);
  }
  // Because the loop will run forever, we never reach this point.
  // As such, the server must be shutdown manually.
  sem_destroy(&boardMutex);
  close(sockfd);
  return 0;
}

// Functions Defined
void error(char* errorMsg){
  perror(errorMsg);
  exit(1);
}

void checkArgs(int numArgs){
  if (numArgs < 2){
    char msg[] = "ERROR: No port provided.";
    error(msg);
  }
}

void checkSocket(int socket){
  if (socket < 0){
    char msg[] = "ERROR: Invalid socket.\n";
    error(msg);
  }
}

void checkStatus(int stat){
  if (stat != 0){
    char msg[] = "ERROR: Bad status.";
    error(msg);
  }
}

void checkRead(int bytes, int socket){
  if (bytes <= 0){
    pthread_detach(pthread_self());
    close(socket);
  }
}

void checkWrite(int writeChars){
  if (writeChars < 0){
    char msg[] = "ERROR: Issue writing to socket.";
    error(msg);
  }
}

void updateBoard(string name, int result){
  int hold; // To swap turn counts.
  string holdName; // To swap player names.
  if (result < leader3 || leader3 == 0){
    leader3 = result;
    name3 = name;
    if (leader3 < leader2 || leader2 == 0){
      hold = leader2;
      holdName = name2;
      leader2 = leader3;
      name2 = name3;
      leader3 = hold;
      name3 = holdName;
      if (leader2 < leader1 || leader1 == 0){
        hold = leader1;
        holdName = name1;
        leader1 = leader2;
        name1 = name2;
        leader2 = hold;
        name2 = holdName;
      }
    }
  }
}

string readName(int socket){
  int n;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);
  n = read(socket,buffer,BUFFER_SIZE - 1);
  checkRead(n, socket);
  printf("New Player: %s",buffer);
  string nameVal = "";
  for (int i = 0; i < n; i++)
    nameVal += buffer[i];
  return nameVal;
}

int readNum(int socket){
  int bytesLeft = sizeof(long);
  long networkInt;
  char *bp = (char *) &networkInt;
  while (bytesLeft) {
    int bytesRecv = recv(socket, bp, bytesLeft, 0);
    checkRead(bytesRecv, socket);
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  int readVal = ntohl(networkInt);
  return readVal;
}

void sendNum(long inNum, int socket){
  int n;
  long networkInt = htonl(inNum);
  int bytesSent = send(socket, (void *) &networkInt, sizeof(long), 0);
  if (bytesSent != sizeof(long))
    exit(-1);
  checkWrite(n);
}

void sendName(string pos, string name, int turns, int socket){
  string sendIt = pos;
  if (turns == 0)
    name = "[Empty]\n";
  for (int i = 0; i < name.length() - 1; i++)
    sendIt += name[i];
  sendIt += ' ';
  if (turns != 0)
    sendIt += (to_string(turns));
  sendIt += '\n';
  int n = send(socket, sendIt.data(), sendIt.size(), 0);
  checkWrite(n);
}

void sendBoard(int socket){
  write(socket, "Leaderboard:\n", 13);
  sendName("1. ", name1, leader1, socket);
  sendName("2. ", name2, leader2, socket);
  sendName("3. ", name3, leader3, socket);
}

void *runClient(void *inClient){
  // First, let's collect the random number and socket.
  ClientArgs *clientVals = (struct ClientArgs *) inClient;
  int clientSock = clientVals->clientSock;
  int correctNum = clientVals->correctNum;
  delete clientVals;

  // Next, we need to read the user's name.
  int n;
  char buffer[BUFFER_SIZE];

  // First, let's get the usernmae of the player.
  string playerName = readName(clientSock);

  // Now, let's play the guessing game!
  int turnCount = 0;
  int guessNum = 0;
  while (guessNum != correctNum){
    // Increment the count and receive the integer.
    turnCount++;
    guessNum = readNum(clientSock);

    // Based on the guess, provide the user with an appropriate response.
    if (guessNum > correctNum)
      write(clientSock, "Too high", 8);
    else if (guessNum < correctNum)
      write(clientSock, "Too low", 7);
    else // (guessNum == correctNum)
      write(clientSock, "Correct guess!", 14);
  }

  // We ensure there is mutual exclusion when we update the leaderboard.
  sem_wait(&boardMutex);
  updateBoard(playerName, turnCount);
  sem_post(&boardMutex);

  // Then, we send the user the current leaderboard standings.
  sendBoard(clientSock);

  pthread_detach(pthread_self());
  printf("Closing a connection on player victory.\n");
  close(clientSock);
  return NULL;
}
