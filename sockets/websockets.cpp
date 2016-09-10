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



Websockets_frame::Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned char OPCODE, bool MASK, size_t PAYLOAD_LENGTH, char* PAYLOAD)
{/* 
Constructs a Websockets_frame object from the parameters.
Returns: a new websockets frame object, ready to be sent.
*/
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
	ptr = NULL;

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
		this->PAYLOAD = (const char *)(data + len);
	}
	len += this->PAYLOAD_LENGTH;
}

int Websockets_frame::send_frame(Websockets_connection * con)
{/* Sends the frame */

	// char test[11] = { /* Packet sending "hello", for debugging */
	//	0x81, 0x85, 0x15, 0x98, 0x74, 0x48, 0x7d, 0xfd,
	//	0x18, 0x24, 0x7a };
	
	// send((*con).s, (const char *) &test, 11, 0);

	 return send((*con).s, (const char *)&frame, len, 0);
}

bool Websockets_frame::fin()
{/* Returns the FIN bit, indicating that this is the final fragment in a message. */
	return this->FIN;
}

bool Websockets_frame::rsv1()
{/* Returns the RSV1 bit, for negotiated extensions. */
	return this->RSV1;
}

bool Websockets_frame::rsv2()
{/* Returns the RSV2 bit, for negotiated extensions. */
	return this->RSV2;
}

bool Websockets_frame::rsv3()
{/* Returns the RSV3 bit, for negotiated extensions. */
	return this->RSV3;
}

bool Websockets_frame::mask()
{/* Returns the MASK bit, indicating whether the "Payload data" is masked. */
	return this->MASK;
}

unsigned char Websockets_frame::opcode()
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

size_t Websockets_frame::payload_length()
{ /* Returns the payload length as defined by the frame header. */
	return this->PAYLOAD_LENGTH;
}

const char * Websockets_frame::payload()
{ /* returns a pointer to the payload */
	return this->PAYLOAD;
}
