/*
websockets.cpp and websockets.h
http://www.github.com/marcelboldt/websockets

Copyright (C) 2016 Marcel Boldt

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

Marcel Boldt <boldt@live.de>

*/


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


class Websockets_frame;

class Websockets_connection { 
public:
	Websockets_connection(const char* server, u_short port, const char* host, const unsigned char* key);
	~Websockets_connection();
	int send_data(char* data, size_t length, uint8_t oc);
	int receive_data(const char* filename);
	int close(uint16_t closecode, Websockets_frame* recv_cf = nullptr); // TODO
	SOCKET s;

private:
	char server_reply[BUFFER_SIZE];
	bool connected = false;
	uint8_t conn_status = 0;
	/* 
	 * 0 = intialised
	 * 1 = establishing
	 * 2 = open
	 * 3 = closing
	 * 4 = closed, cleanly
	 * 5 = closed, not cleanly
	 * 6 = failing
	 * 4 = failed
	 */
	bool client = true; // toggles payload masking
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