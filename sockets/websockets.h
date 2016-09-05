
#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include<stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#include<windows.h>
#include<string.h>
#include<boost/dynamic_bitset.hpp>
#include<bitset>

#include "base64.h"

#define MAX_FRAME_SIZE 32768 // Bit - 32768 = 4 KB


class Websockets_connection {
public:
	Websockets_connection(const char* server, u_short port, const char* host, const unsigned char* key);
	~Websockets_connection();
	SOCKET s;

private:
	bool connected = FALSE;
};


class Websockets_frame {
public:
	Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned int opcode, bool mask, size_t payload_length, std::vector<char>* payload);
	int send_frame(Websockets_connection * con);

private:
	boost::dynamic_bitset<unsigned char> frame;

};

#endif /* websockets.h */