
#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include<stdio.h>
#include<iostream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include<string.h>

#include "base64.h"

#define MAX_FRAME_SIZE 32768 // Bit - 32768 = 4 KB


class Websockets_connection {
public:
	Websockets_connection(const char* server, u_short port, const char* host, const unsigned char* key);
	~Websockets_connection();

	//int send_data(char* data); // TODO
	//char* receive_data(); // TODO
	SOCKET s;

private:
	bool connected = FALSE;
};


class Websockets_frame {
public:
	Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned char OPCODE, bool MASK, size_t payload_length, char* payload);
	Websockets_frame::Websockets_frame(const char * data); // TODO
	int send_frame(Websockets_connection * con);

	bool fin();
	bool rsv1();
	bool rsv2();
	bool rsv3();
	bool mask();
	unsigned char opcode();
	size_t payload_length();
	const char* payload();

private:
	unsigned char frame[MAX_FRAME_SIZE];
	size_t len;
	bool sent = FALSE, FIN, RSV1, RSV2, RSV3, MASK;
	unsigned char OPCODE;
	size_t PAYLOAD_LENGTH;
	const char * PAYLOAD;

};

#endif /* websockets.h */