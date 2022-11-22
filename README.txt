Using the files in hw6.zip.
Created by Casey Nguyen.
Finalized June 3rd, 2021.

server.cpp
To compile: g++ server.cpp -std=c++11 -lpthread -o server
To execute: ./server [port_number]

client.cpp
To compile: g++ client.cpp -std=c++11 -lpthread -o client
To execute: ./social_distancing [hostname] [port_number]
(Note: An error message will appear showing the user this proper usage.)

Functions:
(Generally, one of the integers for most functions is the socket.)

void error(string): Exits the server with a provided message detailing why.
void checkArgs(int): Checks if the arguments provided (the number) is valid.
void checkRead(int): Checks if the read was successful.
void checkWrite(int): Checks if the write was successful.
string readName(int): Gets the name from the client/server.
int readNum(int, int): Ditto, for numbers.
void sendName(string, string, int, int): Sends a name (this specific one is the leaderboard one).
void sendNum(int): Ditto, for numbers.
void *runClient(void*): Runs a thread, letting the game run.

On a side note, the server must be manually exited with CTRL+C. If it is done with CTRL+Z, that port number will can't be released until cs1 is rebooted.
