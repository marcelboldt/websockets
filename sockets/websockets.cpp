#define _CRT_SECURE_NO_WARNINGS

#include "websockets.h"

Websockets_connection::Websockets_connection(const char* ip, u_short port, const char* host, const unsigned char* key)
{ /* Creates socket and initialises a Websocket connection to a remote server.
  Returns: A Websockets_connection object */
	WSADATA wsa;
	s = INVALID_SOCKET;
	struct sockaddr_in server;
	memset(&server, 0, sizeof(server));
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
	strcat(message, "\r\n\r\n");

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d\n", WSAGetLastError());
	}


																			// create socket

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
	}


																			// connect to remote server

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(s, (SOCKADDR *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
	}


																			// send data

	int sr = send(s, message, (int)strlen(message), 0);
	if (sr == SOCKET_ERROR)
	{
		puts("Send failed");
	}

	Sleep(200); // necessary? socket blocking?

																			// Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 20000, 0)) == SOCKET_ERROR)
	{
		puts("recv failed");
	}

	this->connected = TRUE; // TODO: parse the server reply
}

Websockets_connection::~Websockets_connection()
{
	// close & unload winsock
	closesocket(s);
	WSACleanup();
}



Websockets_frame::Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned char opcode, bool mask, size_t payload_length, char* payload)
{/* 
Constructs a Websockets_frame object from the parameters.
Returns: a new websockets frame object, ready to be sent.
*/
	len = 0;

	frame[0] = (FIN) ? (1 << 7) : 0;
	
	frame[0] |= (RSV1) ? (1 << 6) : 0;
	frame[0] |= (RSV2) ? (1 << 5) : 0;
	frame[0] |= (RSV3) ? (1 << 4) : 0;
	frame[0] |= opcode;

	frame[1] = (mask) ? (1 << 7) : 0;

																			// payload_length

	unsigned char * ptr = (unsigned char *) new size_t(htonll(payload_length));
	if (payload_length > (size_t) 125) {
		if (payload_length > (size_t) 65535) { // > 65535 Bit (up to 2^64)
			frame[1] |= (unsigned char) 127;
			
			for (int i = 0; i < 8; i++) {
				frame[2 + i] = *(ptr+i); // all bytes of payload_length
			}
			len = 10;
		}
		else { // 126 - 65535 Bit
			frame[1] |= (unsigned char)126;

			for (int i = 0; i < 2; i++) {
				frame[2 + i] = *(ptr + i+6); // the 7th - 8th Byte (2 LSB Bytes) of payload_length
			}
			len = 4;
		}
	}
	else { // 0 - 125 Bit
		frame[1] |= *(ptr+7); // the 8th Byte of payload_length
		len = 2;
	}
	delete ptr;
	ptr = NULL;

																			// masking_key

	const uint32_t* masking_key = new const uint32_t(rand() % UINT32_MAX);
	//unsigned char masking_key[] = { 0, 0, 0, 0 }; // for debugging

	for (int i = 0; i < 4; i++) {
		frame[len + i] = *(masking_key + i);
	}
	len += 4;


																			// payload

	for (int i = 0; i < payload_length; i++) {
		//  RFC 6455 sec. 5.3:
		//	Octet i of the transformed data("transformed-octet-i") is the XOR (^) of
		//	octet i of the original data("original-octet-i") with octet at index
		//	i modulo (%) 4 of the masking key("masking-key-octet-j")

		frame[len] = *(payload+i) ^ *(unsigned char*)(masking_key + (i % 4));
		len++;
	}
}

Websockets_frame::Websockets_frame(const char * data) {
	/* Parses the data and returns a Websocket frame object.
	*/


}

int Websockets_frame::send_frame(Websockets_connection * con)
{/* Sends the frame */

	// char test[11] = { /* Packet sending "hello", for debugging */
	//	0x81, 0x85, 0x15, 0x98, 0x74, 0x48, 0x7d, 0xfd,
	//	0x18, 0x24, 0x7a };
	
	// send((*con).s, (const char *) &test, 11, 0);

	 return send((*con).s, (const char *)&frame, len, 0);
}

