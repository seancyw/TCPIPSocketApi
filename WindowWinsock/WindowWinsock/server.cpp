#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main()
{
	WSADATA wsaData;
	
	SOCKET listenSocket = INVALID_SOCKET;
	SOCKET clientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int sendResult;
	char receivedBuffer[DEFAULT_BUFLEN];
	int receivedBufferLength = DEFAULT_BUFLEN;

	// Initialize winsock
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error != 0) {
		std::cout << "WSAStartup failed with error: " << error << std::endl;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	error = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (error) {
		std::cout << "getaddrinfo failed with error: " << error << std::endl;
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET) {
		std::cout << "socket failed with error" << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup TCP listening socket
	error = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (error == SOCKET_ERROR) {
		std::cout << "bind failed with error " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	error = listen(listenSocket, SOMAXCONN);
	if (error) {
		std::cout << "listen failed with error " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	clientSocket = accept(listenSocket, NULL, NULL);
	if (clientSocket == INVALID_SOCKET) {
		std::cout << "accept failed with error " << WSAGetLastError() << std::endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// No longer need server socket
	closesocket(listenSocket);

	// Receive until peer shuts down connection
	do {
		error = recv(clientSocket, receivedBuffer, receivedBufferLength, 0);
		if (error > 0) {
			std::cout << "bytes received: " << error << std::endl;

			// echo buffer back to sender
			sendResult = send(clientSocket, receivedBuffer, receivedBufferLength, 0);
			if (sendResult == SOCKET_ERROR) {
				std::cout << "send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(clientSocket);
				WSACleanup();
				return 1;
			}

			std::cout << "bytes send: " << sendResult << std::endl;
		}
		else if(error == 0) {
			std::cout << "Connection closing...\n";
		}
		else {
			std::cout << "received failed with error " << WSAGetLastError() << std::endl;
			closesocket(clientSocket);
			WSACleanup();
			return 1;
		}
	} while (error > 0);

	// shutdown the connection since we are done
	error = shutdown(clientSocket, SD_SEND);
	if (error == SOCKET_ERROR) {
		std::cout << "shutdown failed with error " << WSAGetLastError() << std::endl;
		closesocket(clientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}