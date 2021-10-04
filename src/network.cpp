#include "network.hpp"

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

TCPSocket::TCPSocket()
{
	// AF_INET = IPv4; SOCK_STREAM = TCP
	sock = socket(AF_INET, SOCK_STREAM, 0);

	// Allows for a faster server restart/redeploy
	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	           sizeof(opt));
}

TCPSocket::TCPSocket(int a_sock) : sock(a_sock)
{
	if (sock == -1) {
		std::cerr << "Error: Socket could not be created" << std::endl;
		throw(std::runtime_error("TCPSocket()"));
	}
}

TCPSocket::~TCPSocket() { close(sock); }

// Allows for custom implicit conversion to type int
TCPSocket::operator const int() const { return sock; }
