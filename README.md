# websockets.h

C++ implementation of [RFC 6455](https://tools.ietf.org/html/rfc6455) (The Websocket Protocol).

# Status

Handshake, sending, receiving and parsing frames works so far - so do the higher level methods send_data and receive_data. So 80 % is done...
Currently it only supports winsock, but that gets fixes asap.

