#include "main.h"


void main()
{
	// Initialize Winsock (Windows Sockets).
	WSAData wsaData;
	struct timeval timeout;
	timeout.tv_sec = 120;
	timeout.tv_usec = 0;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}




	// Server side:
	// Create a listen socket for incoming connection.
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}


	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	// Accept connections and handles them one by one.
	while (true)
	{

		fd_set waitRecv, waitSend;
		setupFdSets(waitRecv, waitSend, sockets);


		// select function - to determines the status of one or more sockets
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, &timeout);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}
		handleSocketEvents(waitRecv, waitSend, sockets, nfd);

	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

	
bool addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			sockets[i].lastActive = time(NULL);
			socketsCount++;
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "Time Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "Time Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;
		sockets[index].lastActive = time(NULL);
		int prevlen = sockets[index].len;



		if (sockets[index].len > 0 && sockets[index].send != SEND)
		{
			sockets[index].request.parseRequest(sockets[index].buffer);
			sockets[index].send = SEND;
			//update the buffer
			memcpy(sockets[index].buffer, &sockets[index].buffer[sockets[index].len], BUFFER_SIZE - sockets[index].len);
			//update len
			sockets[index].len -= prevlen;
			
		}

	}

}

void sendMessage(int index)
{
	int bytesSent = 0;
	string sendBuff;

	SOCKET msgSocket = sockets[index].id;

	//update the response object of the socket
	sockets[index].response.UpdateResponse(sockets[index].request);
	//create the send message
	sendBuff = sockets[index].response.getResponse();


	bytesSent = send(msgSocket, sendBuff.c_str(), (int)sendBuff.length(), 0);
	if (SOCKET_ERROR == bytesSent)
	{
		cout << "Time Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "Time Server: Sent: " << bytesSent << "\\" << sendBuff.length()<< " bytes of \"" << sendBuff << "\" message.\n";
	//sockets[index].lastActive = time(NULL);
	sockets[index].send = IDLE;
}


void setupFdSets(fd_set& waitRecv, fd_set& waitSend, struct SocketState* sockets)
{

	FD_ZERO(&waitRecv);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
			FD_SET(sockets[i].id, &waitRecv);
	}

	FD_ZERO(&waitSend);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].send == SEND)
			FD_SET(sockets[i].id, &waitSend);
	}


}

void handleSocketEvents(fd_set& waitRecv, fd_set& waitSend, struct SocketState* sockets,int& nfd)
{
	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, &waitRecv))
		{
			nfd--;
			switch (sockets[i].recv)
			{
			case LISTEN:
				acceptConnection(i);
				break;

			case RECEIVE:
				receiveMessage(i);
				break;
			}
		}
	}

	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, &waitSend))
		{
			nfd--;
			switch (sockets[i].send)
			{
			case SEND:
				sendMessage(i);
				break;
			}
		}
	}


	// Check for timed-out connections
	time_t currentTime = time(NULL);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv != EMPTY && difftime(currentTime, sockets[i].lastActive) > 120 && sockets[i].recv != LISTEN)
		{
			cout << "Server: Connection timed out. Closing socket " << sockets[i].id << endl;
			closesocket(sockets[i].id);
			removeSocket(i);
		}
	}


}