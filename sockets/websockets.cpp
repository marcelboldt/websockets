#define _CRT_SECURE_NO_WARNINGS

#include "websockets.h"

Websockets_connection::Websockets_connection(const char* ip, u_short port, const char* host, const unsigned char* key)
{
	WSADATA wsa;
	struct sockaddr_in server;
	char server_reply[20000];
	char* message = new char[2000];
	char* str2 = new char[200];
	int recv_size;

	// compose message string

	strcpy(message, "GET ws://");
	strcat(message, ip);
	strcat(message, "/ HTTP/1.1\r\nHost: ");
	strcat(message, host);
	strcat(message, "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: ");
	std::string key_b64 = base64_encode(key, 16);
	strcat(message, key_b64.c_str());
	strcat(message, "\r\n");

	printf("%s", message);


	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
	}

	printf("Initialised. ");

	// create socket

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");


	// connect to remote server

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
	}

	puts("Connected");


	// send data

	int sr = send(s, message, (int)strlen(message), 0);
	if (sr == SOCKET_ERROR)
	{
		puts("Send failed");
	}
	printf("Data Send: %i Bytes\n", sr);

	Sleep(200);

	// Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 20000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	puts("Reply received\n");


	server_reply[recv_size] = '\0'; // Add a NULL terminating character to make it a proper string before printing
	str2 = "\nLENGTH: ";
	char* str3;
	str3 = new char[20020];
	strcpy(str3, server_reply);
	strcat(str3, str2);
	printf("%s %i", str3, recv_size);

	this->connected = TRUE; // TODO: parse the server reply
}

Websockets_connection::~Websockets_connection()
{
	// close & unload winsock
	closesocket(s);
	WSACleanup();
}
