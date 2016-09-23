
#include "websockets.h"
#include<iostream>



int send_frame_test()
{
	// Test for connection & send frame
	Websockets_connection* ws = new Websockets_connection("192.168.137.6", 7681, "MBO", reinterpret_cast<const unsigned char *>("asdflökjvnefjaeoönvoiun"));

	char* message = "hello";

	Websockets_frame* wsf = new Websockets_frame(true, false, false, false, 1, true, 5, message);
	(*wsf).send_frame(ws);
	return (*ws).receive_data("test.txt");

}

int frame_parsing_test()
{
	// Test for frame parsing
	char f[] = { /* Packet 317762 -  Server reply to "hello" */
		0x81, 0x10, 0x59, 0x6f, 0x75, 0x20, 0x73, 0x65,
		0x6e, 0x74, 0x20, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
		0x20, 0x0a };

	Websockets_frame* wsf = new Websockets_frame((const char*)&f);

	std::cout << "FIN: " << wsf->fin() << std::endl;
	std::cout << "RSV1: " << wsf->rsv1() << std::endl;
	std::cout << "RSV2: " << wsf->rsv2() << std::endl;
	std::cout << "RSV3: " << wsf->rsv3() << std::endl;
	std::cout << "MASK: " << wsf->mask() << std::endl;
	std::cout << "Opcode: " << int(wsf->opcode()) << std::endl;
	std::cout << "Payload length: " << wsf->payload_length() << std::endl;
	std::cout << "Payload: ";
	auto ptr = wsf->payload();
	for (auto i = 0; i < wsf->payload_length(); i++)
	{
		std::cout << *(ptr + i);
	}
	std::cout << std::endl;

	return 0;
}

int main(int argc, char *argv[])
{

	send_frame_test();
	
	frame_parsing_test();


	return 0;
}