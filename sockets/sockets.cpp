
#include "websockets.h"


int main(int argc, char *argv[])
{

	Websockets_connection* ws = new Websockets_connection("192.168.137.6", 7681, "MBO", reinterpret_cast<const unsigned char *>("asdflökjvnefjaeoönvoiun"));

	char* message = "hello";

	Websockets_frame* wsf = new Websockets_frame(true, false, false, false, 1, true, 5, message);
	(*wsf).send_frame(ws);

	return 0;
}