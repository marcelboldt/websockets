



#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include<stdio.h>
#include<iostream>
#include<fstream>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include<string.h>
#include<vector>

#include "base64.h"

#define MAX_FRAME_SIZE 32768 // Bit - 32768 = 4 KB
#define BUFFER_SIZE 36864 // 4,5 KB

// errors
#define UNKNOWN_OPCODE -11

// todo: send stream


class Websockets_connection { 
public:
	Websockets_connection(const char* server, u_short port, const char* host, const unsigned char* key);
	~Websockets_connection();
	int send_data(char* data, size_t length, uint8_t oc);
	int receive_data(const char* filename); // TODO
	SOCKET s;

private:
	char server_reply[BUFFER_SIZE];
	bool connected = false;
	bool client = true;
};


class Websockets_frame {
public:
	Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned char OPCODE, bool MASK, size_t payload_length, char* payload);
	Websockets_frame(const char * data);
	int send_frame(Websockets_connection * con) const;

	bool fin() const;
	bool rsv1() const;
	bool rsv2() const;
	bool rsv3() const;
	bool mask() const;
	size_t frame_length() const;
	unsigned char opcode() const;
	size_t payload_length() const;
	const char* payload() const;

private:
	unsigned char frame[MAX_FRAME_SIZE];
	size_t len;
	bool sent = false, FIN, RSV1, RSV2, RSV3, MASK;
	unsigned char OPCODE;
	size_t PAYLOAD_LENGTH;
	const char * PAYLOAD;

};

#endif /* websockets.h */