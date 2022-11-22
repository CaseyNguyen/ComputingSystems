// Casey Nguyen
// client.cpp
// 5/31/2021
//
// PURPOSE: This program works in conjunction with the other file in the .zip,
// server.cpp, to run the game "Are You Psychic!". It will attempt to connect
// to the server and, if successful, will ask for a username. Then, the user
// will guess a number of times to see what the value of the mystery number is.
// If performed well (or early), the user will place on a leaderboard. The
// board is shown regardless of performance.
//
// INPUT: hostname, portno, username, guessnum
//
// PROCESSING:
// hostname - Used to attempt to connect to the host server. (CMDLINE)
// portno - Attempts to connect to server with this port number. (CMDLINE)
// username - Sends the name of the player to the server.
// guessnum - Send the guess to the server.
//
// OUPUT:
// hostname/portno - Checks if connection works or not.
// username - Allows them to (possibly) make the leaderboard.
// guessnum - Keeps the turn count so the player can see it (not server-related).
//
// EXAMPLE:
/*
  [nguyencase1@cs1 c3500]$ ./client 10.124.72.20 1024
  Enter your name: Dr. Mario
  Turn: 1
  Enter a guess: 3
  Result of guess: Too low
  Turn: 2
  Enter a guess: 86
  Result of guess: Too high
  Turn: 3
  Enter a guess: 76
  Result of guess: Correct guess!

  Congratulations! It took 3 turns to guess the number!

  Leaderboard:
  1. Dr. Mario 3
  2. [Empty]
  3. [Empty]
  [nguyencase1@cs1 c3500]$ ./client 10.124.72.20 1024
  Enter your name: Kaede Akamatsu
  Turn: 1
  Enter a guess: 64
  Result of guess: Too low
  Turn: 2
  Enter a guess: 9032
  Invalid guess. Re-enter a value within 0 to 999: 460
  Result of guess: Correct guess!

  Congratulations! It took 2 turns to guess the number!

  Leaderboard:
  1. Kaede Akamatsu 2
  2. Dr. Mario 3
  3. [Empty]
*/
// This one too. Lots of libraries...
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
using namespace std;

const int BUFFER_SIZE = 100; // The size of the buffer used to read in data.

// Also shown in the readme document.
void error(char*);
void checkArgs(int);
void sendName(int);
long getGuess();
void sendGuess(int);
void checkRead(int);
int readNum(int);
void printBoard(int);

int main(int argc, char *argv[]){
  // Lots of mumbo jumbo to get the connection to the server.
  checkArgs(argc);
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[BUFFER_SIZE];
  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    char msg[] = "ERROR: Issue opening socket.\n";
    error(msg);
  }
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr,"ERROR: No such host.\n");
    exit(0);
  }
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);
  if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
    char msg[] = "ERROR: Issue connecting.\n";
    error(msg);
  }

  // First, let's send the server the player's username.
  sendName(sockfd);

  // Now, we can play the game.
  int turnCount = 0; // Tracks the number of turns taken for the client.
  while(n != 14){ // The victory message is 14 characters long.
    // First, we increment the turn count and send a guess.
    turnCount++;
    printf("Turn: %d\n", turnCount);
    sendGuess(sockfd);

    // Then, we read in the response from the server.
    bzero(buffer,BUFFER_SIZE);
    n = read(sockfd,buffer,BUFFER_SIZE - 1);
    if (n < 0){
      char msg[] = "ERROR: Issue reading from socket.\n";
      error(msg);
    }
    printf("Result of guess: %s\n",buffer);
  }
  printf("\nCongratulations! It took %d turn(s) to guess the number!\n\n", turnCount);
  printBoard(sockfd); // We show the user the current leaderboard.
  close(sockfd);
  return 0;
}

// Functions defined
void error(char* errorMsg){
  perror(errorMsg);
  exit(0);
}

void checkArgs(int numArgs){
  if (numArgs < 3){
    char msg[] = "Usage: ./client [hostname] [port number]\n";
    error(msg);
  }
}

void checkRead(int readChars){
  if (readChars < 0){
    char msg[] = "ERROR: Issue reading from socket.";
    error(msg);
  }
}

void sendName(int socket){
  int n;
  char buffer[BUFFER_SIZE];
  bzero(buffer,BUFFER_SIZE);
  printf("Enter your name: ");
  fgets(buffer,BUFFER_SIZE - 1,stdin); // It will only send the first 100 chars.
  n = write(socket,buffer,strlen(buffer));
  if (n < 0){
    char msg[] = "ERROR: Issue writing to socket.\n";
    error(msg);
  }
}

long getGuess(){
  printf("Enter a guess: ");
  long guess;
  cin >> guess;
  while (guess < 0 || guess > 999){
    printf("Invalid guess. Re-enter a value within 0 to 999: ");
    cin >> guess;
  }
  return guess;
}

void sendGuess(int socket){
  int sendChars;
  long hostInt = getGuess();
  long networkInt = htonl(hostInt);
  int bytesSent = send(socket, (void *) &networkInt, sizeof(long), 0);
  if (bytesSent != sizeof(long))
    exit(-1);
  if (sendChars < 0){
    char msg[] = "ERROR: Issue writing to socket.\n";
    error(msg);
  }
}

int readNum(int socket){
  int bytesLeft = sizeof(long);
  long networkInt;
  char *bp = (char *) &networkInt;
  while (bytesLeft) {
    int bytesRecv = recv(socket, bp, bytesLeft, 0);
    if (bytesRecv <= 0)
      exit(-1);
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  int readVal = ntohl(networkInt);
  return readVal;
}

string readName(int socket){
  string inString;
  int n;
  char buffer[BUFFER_SIZE];
  while((n = recv(socket, buffer, sizeof(buffer), 0)) > 0)
    inString.append(buffer, buffer + n);
  return inString;
}

void printBoard(int socket){
  for (int i = 0; i < 4; i++)
    cout << readName(socket);
}