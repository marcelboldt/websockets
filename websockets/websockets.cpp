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


#define _CRT_SECURE_NO_WARNINGS

#include <iomanip>
#include "websockets.h"

Websockets_connection::Websockets_connection(const char *ip, uint16_t port, const char *host)
{ /* Creates socket and initialises a Websocket connection to a remote server.
  Returns: A Websockets_connection object */


	s = INVALID_SOCKET;
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
	char* message = new char[2000];

	key = new unsigned char[16];
	for (int i = 0; i < 16; i++) {
		key[i] = (unsigned char) rand() % 255;
	}
	// compose message string

	strcpy(message, "GET ws://");
	strcat(message, ip);
	strcat(message, "/ HTTP/1.1\r\nHost: ");
	strcat(message, host);
	strcat(message, "\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Version: 13\r\nSec-WebSocket-Key: ");
	std::string key_b64 = base64_encode(key, 16);
	strcat(message, key_b64.c_str());
	strcat(message, "\r\n\r\n");
#ifdef _WIN32
	throw std::runtime_error("WIN32");
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Winsock startup failed. Error Code : %d\n", WSAGetLastError());
        throw winsock_startup_error();
    }
#endif


	// create socket

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
#ifdef _WIN32
        printf("Error at socket(): %ld\n", WSAGetLastError());
                WSACleanup();
#else
		printf("Error at socket(): %d\n", errno);
#endif
		throw socket_create_error();
	}


	// connect to remote server

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(s, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		throw socket_connect_error();
	}


	// send data

	if (send(s, message, (int)strlen(message), 0) == SOCKET_ERROR)
	{
		throw socket_send_error();
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(RECV_DELAY));

	// Receive a reply from the server
	if ((recv(s, server_reply, BUFFER_SIZE, 0)) == SOCKET_ERROR)
	{
		throw socket_recv_error();
	}

	this->CONNECTED = true; // TODO: parse the server reply
}

Websockets_connection::~Websockets_connection()
{
#ifdef _WIN32
	// close & unload winsock
	closesocket(s);
	WSACleanup();
#endif
}

int Websockets_connection::send_data(const char *data, size_t length, uint8_t oc)
{/* Sends the data given over the open Websockets connection.
	Returns: an the number of frames sent if successful, otherwise -1.
	Operation codes:
	* 0 : continuation
	* 1 : text data
	* 2 : binary data
	* 3-7 reserved non-control
	* 8 : connection close
	* 9 : ping
	* A : pong
	* B-F reserved control
 */

	auto i = 0;
	std::vector<bool> succ;
	Websockets_frame* f;

	while(i + MAX_FRAME_SIZE < length)
	{
		f = new Websockets_frame(false, false, false, false, (i == 0 ? oc : 0), (this->client ? 1 : 0), MAX_FRAME_SIZE, (data + i));
		succ.push_back(f->send_frame(this));
		i += MAX_FRAME_SIZE;
		delete(f);
	}
	f = new Websockets_frame(true, false, false, false, (i == 0 ? oc : 0), (this->client ? 1 : 0), length, (data + i));
	succ.push_back(f->send_frame(this));
	delete(f);

	for (i = 0; i < succ.size(); i++)
	{
		if (!succ[i]) return -1;
	}

	return succ.size();
}

int Websockets_connection::receive_data(const char *filename, bool append)
{/* Reads Websockets frames from the socket and writes data into a file.
	Returns if successful the amount of bytes written; otherwise an error code as defined in websockets.h
	*/
	if (!append) {
		std::remove(filename);
	}

	//Websockets_frame* f = new Websockets_frame(this->s);

	std::shared_ptr<Websockets_frame> f(new Websockets_frame(this->s));
	auto len = f->payload_length();
	while (!f->fin()) {
		// f = new Websockets_frame(this->s, f->payload());
		std::shared_ptr<Websockets_frame> f(new Websockets_frame(this->s, f->payload()));
		len += f->payload_length();
	}
	f->save_payload(filename);

	return len;
}

int Websockets_connection::close(uint16_t closecode, Websockets_frame* recv_cf)
{ /* Closes the connection. */

	//this->connection state CLOSING
	//send a close control frame with closecode


	char* payload = new char[20];
	strcpy(payload, (const char*)closecode);
	strcat(payload, " Goodbye...");

	Websockets_frame* cf = new Websockets_frame(true, false, false, false, 0x8, true,sizeof(payload), payload);
	cf->send_frame(this);

	//  if init then wait for receiving a close frame

	// shutdown() with SHUT_WR
	//recv until 0
	// close()
	// state -> closed

	return 0;

}

bool Websockets_connection::connected() {
	return this->CONNECTED;
}

Websockets_frame::Websockets_frame(bool FIN, bool RSV1, bool RSV2, bool RSV3, unsigned char OPCODE, bool MASK,
								   size_t PAYLOAD_LENGTH, const char *PAYLOAD) {
	/*
Constructs a Websockets_frame object from the parameters.
Returns: a new websockets frame object, ready to be sent.
*/

	this->FIN = FIN;
	this->RSV1 = RSV1;
	this->RSV2 = RSV2;
	this->RSV3 = RSV3;
	this->OPCODE = OPCODE;
	this->MASK = MASK;
	this->PAYLOAD_LENGTH = PAYLOAD_LENGTH;
	this->PAYLOAD = PAYLOAD;


	len = 0;

	frame[0] = (FIN) ? (1 << 7) : 0;

	frame[0] |= (RSV1) ? (1 << 6) : 0;
	frame[0] |= (RSV2) ? (1 << 5) : 0;
	frame[0] |= (RSV3) ? (1 << 4) : 0;
	frame[0] |= OPCODE;

	frame[1] = (MASK) ? (1 << 7) : 0;

	// PAYLOAD_LENGTH

	unsigned char * ptr = (unsigned char *) new size_t(htonll(PAYLOAD_LENGTH));
	if (PAYLOAD_LENGTH > (size_t) 125) {
		if (PAYLOAD_LENGTH > (size_t) 65535) { // > 65535 Bit (up to 2^64)
			frame[1] |= (unsigned char) 127;

			for (int i = 0; i < 8; i++) {
				frame[2 + i] = *(ptr+i); // all bytes of PAYLOAD_LENGTH
			}
			len = 10;
		}
		else { // 126 - 65535 Bit
			frame[1] |= (unsigned char)126;

			for (int i = 0; i < 2; i++) {
				frame[2 + i] = *(ptr + i+6); // the 7th - 8th Byte (2 LSB Bytes) of PAYLOAD_LENGTH
			}
			len = 4;
		}
	}
	else { // 0 - 125 Bit
		frame[1] |= *(ptr+7); // the 8th Byte of PAYLOAD_LENGTH
		len = 2;
	}
	delete ptr;
	ptr = nullptr;

	// masking_key

	const uint32_t* masking_key = new const uint32_t(rand() % UINT32_MAX);
	//unsigned char masking_key[] = { 0, 0, 0, 0 }; // for debugging

	for (int i = 0; i < 4; i++) {
		frame[len + i] = *(masking_key + i);
	}
	len += 4;


	// payload

	for (int i = 0; i < PAYLOAD_LENGTH; i++) {
		//  RFC 6455 sec. 5.3:
		//	Octet i of the transformed data("transformed-octet-i") is the XOR (^) of
		//	octet i of the original data("original-octet-i") with octet at index
		//	i modulo (%) 4 of the masking key("masking-key-octet-j")

		frame[len] = *(PAYLOAD+i) ^ *(unsigned char*)(masking_key + (i % 4));
		len++;
	}
}

Websockets_frame::Websockets_frame(const char * data) {
	/* Parses the data and returns a Websocket frame object.
	*/
	len = 0;
	// 1st Byte
	this->FIN = (*data & 128) > 0;
	this->RSV1 = (*data & 64) > 0;
	this->RSV2 = (*data & 32) > 0;
	this->RSV3 = (*data & 16) > 0;
	this->OPCODE = (*data & 15);
	// 2nd Byte
	this->MASK = (*(data+1) & 128) > 0;

	char c = *(data + 1) & 127;
	if (c < 126) { // this is payload_length
		this->PAYLOAD_LENGTH = c;
		len = 2;
	}
	else if (c == 126) {
		// next two bytes are payload_length
		this->PAYLOAD_LENGTH = ntohs(*(uint16_t*)(data +2));
		len = 4;
	}
	else {
		// next 8 bytes are payload length
		this->PAYLOAD_LENGTH = ntohll(*(uint64_t*)(data + 2));
		len = 10;
	}

	// Masking key & Payload
	if (this->MASK) {
		const char* masking_key = (data + len);
		len += 4;

		char * pl = new char[this->PAYLOAD_LENGTH];
		for (size_t i=0; i < this->PAYLOAD_LENGTH; i++) {
			*(pl + i) = *(data + len + i) ^ *(masking_key + (i % 4));  // unmask
		}

	}
	else {
		this->PAYLOAD = static_cast<const char *>(data + len);
	}
	len += this->PAYLOAD_LENGTH;
}

#ifdef _WIN32
Websockets_frame::Websockets_frame(SOCKET socket, const char * filename) {
#else

Websockets_frame::Websockets_frame(int socket, const char *filename) {
#endif
	/* Reads a frame from a socket and writes the payload into a file while keeping the metadata in the object. If a filename of an existing file is given, the payload is appended.
     * Caution: The file is removed on object destruction - so better do frame->save_payload() first!
    */
	srand(
			std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::system_clock::now().time_since_epoch()).count()
	);

	char *data = new char[BUFFER_SIZE];
	char *fn = new char[128];
	if (filename == nullptr) {
		time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		strftime(fn, 128, "WS frame %Y-%m-%d %X", std::localtime(&tt));
		sprintf(fn, "%s %i.tmp", fn, rand() % 65536);
	} else {
		fn = (char *) filename;
	}

	std::ofstream tfile;
	auto len = 0, recv_len = 0, write_len = 0;

	recv_len = recv(socket, data, BUFFER_SIZE, 0); // receive the first frame

	// 1st Byte
	this->FIN = (*data & 128) > 0;
	this->RSV1 = (*data & 64) > 0;
	this->RSV2 = (*data & 32) > 0;
	this->RSV3 = (*data & 16) > 0;
	this->OPCODE = (*data & 15);
	// 2nd Byte
	this->MASK = (*(data + 1) & 128) > 0;

	char c = *(data + 1) & 127;
	if (c < 126) { // this is payload_length
		this->PAYLOAD_LENGTH = c;
		len = 2;
	} else if (c == 126) {
		// next two bytes are payload_length
		this->PAYLOAD_LENGTH = ntohs(*(uint16_t *) (data + 2));
		len = 4;
	} else {
		// next 8 bytes are payload length
		this->PAYLOAD_LENGTH = ntohll(*(uint64_t *) (data + 2));
		len = 10;
	}

	switch (this->OPCODE) {
/*	case 0:
		for (auto i = 0; i < f->payload_length(); i++) {
			tfile << *(f->payload() + i);
		}
		len += f->payload_length();
		break; */
		case 1:
			tfile.open(fn, std::ios::app);
			break;
		case 2:
			tfile.open(fn, std::ios::app | std::ios::binary);
			break;
		default:
			throw ws_unknown_opcode();
	}

	this->PAYLOAD = fn;

	// Masking key & Payload
	if (this->MASK) {
		const char *masking_key = (data + len);
		len += 4;

		for (auto i = len; i < recv_len; i++) {
			tfile << (*(data + i) ^ *(masking_key + (i % 4)));  // unmask and write to file
		}
		write_len = recv_len - len;

		while (write_len <
			   this->PAYLOAD_LENGTH) { // if remaining frame length < payload length, then read further from socket
			//TODO: add a timeout
			std::this_thread::sleep_for(std::chrono::milliseconds(RECV_DELAY));
			recv_len = recv(socket, data, BUFFER_SIZE, 0);
			if (recv_len == 0) throw socket_unexp_close();
			for (auto i = 0; i < recv_len; i++) {
				tfile << (*(data + i) ^ *(masking_key + ((write_len + i) % 4)));  // unmask and write to file
			}
			write_len = write_len + recv_len;
		}
	} else { // write to file, but not unmask
		for (auto i = len; i < recv_len; i++) {
			tfile << *(data + i);  // write to file
		}
		write_len = recv_len - len;

		while (write_len < this->PAYLOAD_LENGTH) {
			std::this_thread::sleep_for(std::chrono::milliseconds(RECV_DELAY));
			recv_len = recv(socket, data, BUFFER_SIZE, 0);
			if (recv_len == 0) throw socket_unexp_close();
			for (auto i = 0; i < recv_len; i++) {
				tfile << *(data + i);  // write to file
			}
			write_len = write_len + recv_len;
		}
	}
	tfile.close();
	this->PAYLOAD_FILE = true;
}


int Websockets_frame::send_frame(Websockets_connection * con) const
{/* Sends the frame.
	Returns: an int indicating the frame was successfully sent (the return values of winsock / POSIX) send() */

	// char test[11] = { /* Packet sending "hello", for debugging */
	//	0x81, 0x85, 0x15, 0x98, 0x74, 0x48, 0x7d, 0xfd,
	//	0x18, 0x24, 0x7a };

	// send((*con).s, (const char *) &test, 11, 0);

	return send((*con).s, (const char *) &frame, len, 0);
}

bool Websockets_frame::fin() const
{/* Returns the FIN bit, indicating that this is the final fragment in a message. */
	return this->FIN;
}

bool Websockets_frame::rsv1() const
{/* Returns the RSV1 bit, for negotiated extensions. */
	return this->RSV1;
}

bool Websockets_frame::rsv2() const
{/* Returns the RSV2 bit, for negotiated extensions. */
	return this->RSV2;
}

bool Websockets_frame::rsv3() const
{/* Returns the RSV3 bit, for negotiated extensions. */
	return this->RSV3;
}

bool Websockets_frame::mask() const
{/* Returns the MASK bit, indicating whether the "Payload data" is masked. */
	return this->MASK;
}

size_t Websockets_frame::frame_length() const
{/* Returns the length of the frame in Bytes */
	return this->len;
}

unsigned char Websockets_frame::opcode() const
{/* Returns the Opcode:
	*  %x0 denotes a continuation frame
	*  %x1 denotes a text frame
	*  %x2 denotes a binary frame
	*  %x3 - 7 are reserved for further non - control frames
	*  %x8 denotes a connection close
	*  %x9 denotes a ping
	*  %xA denotes a pong
	*  %xB - F are reserved for further control frames
*/
	return this->OPCODE;
}

size_t Websockets_frame::payload_length() const
{ /* Returns the payload length as defined by the frame header. */
	return this->PAYLOAD_LENGTH;
}

const char * Websockets_frame::payload() const
{ /* returns a pointer to the payload */
	return this->PAYLOAD;
}

bool Websockets_frame::payload_file() const {
	return this->PAYLOAD_FILE;
}

Websockets_frame::~Websockets_frame() {

	if (this->PAYLOAD_FILE) {
		if (remove(this->PAYLOAD) != 0) throw ws_tempfile_delete();
	}
}

int Websockets_frame::save_payload(const char *filename) {

	if (this->PAYLOAD_FILE) {
		std::ifstream src(this->PAYLOAD, std::ios::binary);
		std::ofstream dst(filename, std::ios::binary);

		dst << src.rdbuf();
		dst.close();
	} else {
		std::ofstream dst(filename, std::ios::binary);
		dst << this->PAYLOAD;
		dst.close();
	}

	return 0;
}
