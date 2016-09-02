/*
initialise winsock
*/

#include<stdio.h>
#include<winsock2.h>
#include<windows.h>
#include<string.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library

#include "base64.h"




#ifndef _WIN32
// IPv4 AF_INET sockets:
struct sockaddr_in {
	short            sin_family;   // e.g. AF_INET, AF_INET6
	unsigned short   sin_port;     // e.g. htons(3490)
	struct in_addr   sin_addr;     // see struct in_addr, below
	char             sin_zero[8];  // zero this if you want to
};

typedef struct in_addr {
	union {
		struct {
			u_char s_b1, s_b2, s_b3, s_b4;
		} S_un_b;
		struct {
			u_short s_w1, s_w2;
		} S_un_w;
		u_long S_addr;
	} S_un;
} IN_ADDR, *PIN_ADDR, FAR *LPIN_ADDR;


struct sockaddr {
	unsigned short    sa_family;    // address family, AF_xxx
	char              sa_data[14];  // 14 bytes of protocol address
};
#endif



int main(int argc, char *argv[])
{

	// init

	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char server_reply[20000];
	char msg[2000] = "GET ws://192.168.137.6/ HTTP/1.1\r\nHost: MBO\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: ";
	char* message = msg;
	char* str2 = new char[200];
	int recv_size;
	const unsigned char* key = reinterpret_cast<const unsigned char *>("asdfvnöoreajnvjds");

	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}

	printf("Initialised. ");

	// create socket

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");

	// connect to remote server

	server.sin_addr.s_addr = inet_addr("192.168.137.6");
	server.sin_family = AF_INET;
	server.sin_port = htons(7681);

	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");

	Sleep(2000);

	// send data

	std::string key_b64 = base64_encode(key, 16);
	strcat(message, key_b64.c_str());
	strcat(message, "\r\n");

	int sr = send(s, message, strlen(message), 0);
	if (sr == SOCKET_ERROR)
	{
		puts("Send failed");
		return 1;
	}
	printf("Data Send: %i Bytes\n", sr);

	Sleep(2000);

	//Receive a reply from the server
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


	// close & unload winsock
	closesocket(s);
	WSACleanup();

	return 0;
}