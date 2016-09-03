
#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include<stdio.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include<winsock2.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library#pragma once
#include<windows.h>
#include<string.h>
//#include<bitset>


#include "base64.h"


class Websockets_connection {
public:
	Websockets_connection(const char* server, u_short port, const char* host, const unsigned char* key);
	~Websockets_connection();

private:
	bool connected = FALSE;
	SOCKET s;
};

#endif /* websockets.h */