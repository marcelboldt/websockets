
#define _CRT_SECURE_NO_WARNINGS
#include "websockets.h"



int main(int argc, char *argv[])
{

	Websockets_connection* ws = new Websockets_connection("192.168.137.6", 7681, "MBO", reinterpret_cast<const unsigned char *>("asdflökjvnefjaeoönvoiun"));

	Sleep(2000);

	return 0;
}