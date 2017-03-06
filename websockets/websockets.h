/*
websockets.cpp and websockets.h
http://www.github.com/marcelboldt/websockets

 The MIT License (MIT)

Copyright (C) 2016 Marcel Boldt / EXASOL

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Marcel Boldt <marcel.boldt@exasol.com>

*/


#ifndef WEBSOCKETS_H
#define WEBSOCKETS_H

#include<stdio.h>
#include<iostream>
#include<fstream>
#include<exception>

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib") //Winsock Library
#else

#include <sys/socket.h>
#include<arpa/inet.h>
// see http://stackoverflow.com/questions/3022552/is-there-any-standard-htonl-like-function-for-64-bits-integers-in-c

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#include<errno.h>
#endif

#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))

#include <thread>         // std::this_thread::
#include <ctime>
#include <chrono>         // std::chrono::seconds
#include <string.h>
#include <sstream>
#include <vector>
#include <memory>

#include "../base64/base64.h"

#define MAX_FRAME_SIZE 32768 // Bit - 32768 = 4 KB
#define BUFFER_SIZE 36864 // 4,5 KB
#define RECV_DELAY 2 // ms

// todo: send stream

struct socket_connect_error : public std::exception {
    const char * what() const throw () {
        return "Error connecting to socket";
    }
};

struct socket_send_error : public std::exception {
    const char * what() const throw () {
        return "Error sending via socket";
    }
};

struct socket_recv_error : public std::exception {
    const char * what() const throw () {
        return "Error receiving from socket";
    }
};

struct ws_unknown_opcode : public std::exception {
    const char * what() const throw () {
        return "Error: websockets frame with unknown opcode received";
    }
};

struct socket_unexp_close : public std::exception {
    const char * what() const throw () {
        return "Error: socket closed unexpectedly";
    }
};

struct ws_tempfile_delete : public std::exception {
    const char * what() const throw () {
        return "Error: could not remove tempfile";
    }
};

struct socket_create_error : public std::exception {
    const char * what() const throw () {
        return "Error: could not create socket";
    }
};

struct winsock_startup_error : public std::exception {
    const char * what() const throw () {
        return "Error: winsock could not be started";
    }
};

class Websockets_frame;

class Websockets_connection {
public:
    static void write_msg_to_file(std::string msg, std::string name = "errorfile.txt") {
        std::ofstream outfile;
        outfile.open(name, std::ios::app);
        outfile << "Error: " << msg << std::endl;
        outfile.close();
    };

	Websockets_connection(const char *server, uint16_t port, const char *host);
	~Websockets_connection();

	bool connected();

	int send_data(const char *data, size_t length, uint8_t oc);

	int receive_data(const char *filename, bool append = true);
	int close(uint16_t closecode, Websockets_frame* recv_cf = nullptr); // TODO
#ifdef _WIN32
	SOCKET s;
#else
	int s;
#endif


private:
	char server_reply[BUFFER_SIZE]; // this is the server part of the handshake. later parse...
	bool CONNECTED = false;
	unsigned char *key;
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
	Websockets_frame(bool FIN, bool RSV1, bool RSV2, bool RSV3, unsigned char OPCODE, bool MASK, size_t payload_length,
					 const char *payload);
	Websockets_frame(const char * data);

#ifdef _WIN32
	Websockets_frame(SOCKET socket, const char * filename = nullptr);
#else

	Websockets_frame(int socket, const char *filename = nullptr);

#endif
	int send_frame(Websockets_connection * con) const;

	int save_payload(const char *filename);

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
	bool PAYLOAD_FILE = false;
public:
	bool payload_file() const;

	virtual ~Websockets_frame();

};

#endif /* websockets.h */
