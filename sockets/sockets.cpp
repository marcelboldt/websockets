
#include "websockets.h"


int main(int argc, char *argv[])
{

	Websockets_connection* ws = new Websockets_connection("192.168.137.6", 7681, "MBO", reinterpret_cast<const unsigned char *>("asdflökjvnefjaeoönvoiun"));

	std::string msg = "hello";
	std::vector<char> message(msg.begin(), msg.end());
	std::vector<char> extl;

	//(bool  FIN, bool  RSV1, bool RSV2, bool RSV3, unsigned int * opcode, bool mask, size_t payload_length, std::vector<char> * ext_load, std::vector<char> * payload);

	Websockets_frame* wsf = new Websockets_frame(true, false, false, false,(unsigned int) 1, true, message.size(), &message);

	(*wsf).send_frame(ws);

	return 0;
}