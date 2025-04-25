#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include "HttpRequest.h"  
#include "HttpResponse.h" 

#define BUFFER_SIZE 1000

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	int sendSubType;	// Sending sub-type
	char buffer[BUFFER_SIZE];
	int len;
	time_t lastActive;
	HttpRequest request;      // Object to manage incoming request data
	HttpResponse response;    // Object to manage outgoing response data
};

const int TIME_PORT = 27015;
const int MAX_SOCKETS = 60;
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;

bool addSocket(SOCKET id, int what);
void removeSocket(int index);
void acceptConnection(int index);
void receiveMessage(int index);
void sendMessage(int index);
void setupFdSets(fd_set& waitRecv, fd_set& waitSend, struct SocketState* sockets);
void handleSocketEvents(fd_set& waitRecv, fd_set& waitSend, struct SocketState* sockets, int& nfd);

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;