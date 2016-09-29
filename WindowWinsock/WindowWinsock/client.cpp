#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

int main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET connectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*pointer = NULL,
		hints;
	char * sendBuffer = "this is a test";
	char receivedBuffer[DEFAULT_BUFLEN];
	int receivedBufferLength = DEFAULT_BUFLEN;

	// Validate parameters;
	//if (argc != 2) {
	//	std::cout << "usage " << argv[0] << "server-name\n";
	//	return 1;
	//}

	// Initialize winsock
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error) {
		std::cout << "WSAStartup failed with error: " << error;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	// Resolve server address and port
	error = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (error) {
		std::cout << "getaddrinfo failed with error " << error << std::endl;
		WSACleanup();
		return 1;
	}

	// Attempt to connect an address until one succeeds
	for (pointer = result; pointer != NULL; pointer = pointer->ai_next) {

		// Create a SOCKET for connecting to server
		connectSocket = socket(pointer->ai_family, pointer->ai_socktype, pointer->ai_protocol);
		if (connectSocket == INVALID_SOCKET) {
			std::cout << "socket failed with error " << WSAGetLastError() << std::endl;
			WSACleanup();
			return 1;
		}

		// Connect to server
		error = connect(connectSocket, pointer->ai_addr, (int)pointer->ai_addrlen);
		if (error == SOCKET_ERROR) {
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET) {
		std::cout << "unable to connect to server!\n";
		WSACleanup();
		return 1;
	}

	// Send an initial buffer
	error = send(connectSocket, sendBuffer, (int)strlen(sendBuffer), 0);
	if (error == SOCKET_ERROR) {
		std::cout << "send failed with error " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "bytes sent: " << error << std::endl;

	// Shutdown connection since no more data will be sent
	error = shutdown(connectSocket, SD_SEND);
	if (error == SOCKET_ERROR) {
		std::cout << "shutdown failed with error " << WSAGetLastError() << std::endl;
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// Received until the peer closes the connection
	do {
		error = recv(connectSocket, receivedBuffer, receivedBufferLength, 0);
		if (error > 0) {
			std::cout << "bytes received: " << error << std::endl;
		}
		else if (error == 0) {
			std::cout << "connection closed\n";
		}
		else {
			std::cout << "recv failed with error " << WSAGetLastError() << std::endl;
		}
	} while (error > 0);

	// cleanup
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}