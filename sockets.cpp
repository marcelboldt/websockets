
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



#include "websockets/websockets.h"
#include<iostream>


int send_frame_test() {
	// Test for connection & send frame
    Websockets_connection *ws = new Websockets_connection("192.168.137.6", 7681, "MBO");

    const char *message = "012345678901234567890123456789";

    Websockets_frame *wsf = new Websockets_frame(true, false, false, false, 1, true, 30, message);
	(*wsf).send_frame(ws);
    std::string data;
    int len = ws->receive_data(&data);
    std::cout << data << std::endl;
    return len;
}

int send_data_test(const char *message, int len) {
    // Test for connection & send frame
    Websockets_connection *ws = new Websockets_connection("192.168.137.6", 7681, "MBO");

    ws->send_data(message, len, 1);

    std::string data;
    len = ws->receive_data(&data);
    std::cout << "RECV: " << data << std::endl;
    return len;
}

int frame_parsing_test()
{
	// Test for frame parsing
    unsigned char f[] = { /* Packet 317762 -  Server reply to "hello" */
            0x81, 0x10, 0x59, 0x6f, 0x75, 0x20, 0x73, 0x65,
            0x6e, 0x74, 0x20, 0x68, 0x65, 0x6c, 0x6c, 0x6f,
            0x20, 0x0a};

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

    //send_frame_test();

    const char *message = "AFNÖJNFÖKJNRÖKFJANFÖREANFJGHJNFÖOIFÖONÖOREAINFEARÖOIGFNREAGÖONFRJSANFÖLKREANFÖREAONAÖÖLFNFASDFASDLÖFKJAÖLDSKFJAÖLSDKFJAÖLSDKFJAÖSLDKFJÖASLDKFJÖALSDKFJAÖLSDKFJAÖSLDKJF";
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    send_data_test(message, 180);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() / 1000 << " ms."
              << std::endl;


//	frame_parsing_test();


	return 0;
}