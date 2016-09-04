#define _CRT_SECURE_NO_WARNINGS

#include "websockets.h"

Websockets_connection::Websockets_connection(const char* ip, u_short port, const char* host, const unsigned char* key)
{
	WSADATA wsa;
	struct sockaddr_in server;
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
	strcat(message, "\r\n");

	printf("%s", message);


	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
	}

	printf("Initialised. ");

	// create socket

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}

	printf("Socket created.\n");


	// connect to remote server

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons(port);

	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		puts("connect error");
	}

	puts("Connected");


	// send data

	int sr = send(s, message, (int)strlen(message), 0);
	if (sr == SOCKET_ERROR)
	{
		puts("Send failed");
	}
	printf("Data Send: %i Bytes\n", sr);

	Sleep(200);

	// Receive a reply from the server
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

	this->connected = TRUE; // TODO: parse the server reply
}

Websockets_connection::~Websockets_connection()
{
	// close & unload winsock
	closesocket(s);
	WSACleanup();
}

template<size_t s1, size_t s2> // inject bitset s2 into bitset s1 at position start
void inject(std::bitset<s1>& bs1, const std::bitset<s2>& bs2, int* start)
{
	for (size_t i = 0; i<s2; i++)
		bs1[i + start] = bs2[i];
	start += s2;
}

template<size_t s2> // inject bitset s2 into bitset s1 at position start
void inject(boost::dynamic_bitset<>& bs1, const std::bitset<s2>& bs2, int start)
{
	if (start + s2 > bs1.size()) {
		// allocate more bits
		bs1.resize(start + s2)
	}
	for (size_t i = 0; i<s2; i++)
		bs1[i + start] = bs2[i];
}





Websockets_frame::Websockets_frame(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned int * opcode, bool mask, const uint64_t * payload_length, long * masking_key, std::vector<char> * ext_load, std::vector<char> * payload)
{
	frame = boost::dynamic_bitset<>(4);

	frame[0] = FIN;
	frame[1] = RSV1;
	frame[2] = RSV2;
	frame[3] = RSV3;


	std::bitset<4> oc(*opcode);
	inject<4>(frame, oc,frame.size();
	frame.push_back(mask);

	if (*payload_length > (const uint64_t *) 125) {
		if (*payload_length > (uint64_t *) 65535) { // > 65535 Bit (up to 2^64)
			std::bitset<7> pl1(127);
			inject<7>(frame, pl1, frame.size());

			std::bitset<64> pl2(*payload_length);
			inject<64>(frame, pl2, frame.size());
		}
		else { // 126 - 65535 Bit
			std::bitset<7> pl1(126);
			inject<7>(frame, pl1, frame.size());

			std::bitset<16> pl2(*payload_length);
			inject<16>(frame, pl2, frame.size());
		}
	}
	else { // 0 - 125 Bit
		std::bitset<7> pl1(*payload_length);
		inject<7>(frame, pl1, frame.size());
	}

	std::bitset<32> mk(*masking_key);
	inject<32>(frame, mk, frame.size());

	// ext_load
	for (int i = 0; i <= (*ext_load).size(); i++) {
		std::bitset<8> el((*ext_load)[i]);
		inject<8>(frame, el, frame.size());
	}

	// payload
	for (int i = 0; i <= (*payload).size(); i++) {
		std::bitset<8> pl((*payload)[i]);
		inject<8>(frame, pl, frame.size());
	}
	
}

