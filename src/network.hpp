#pragma once

struct TCPSocket {
	int sock;

	TCPSocket();
	TCPSocket(int a_sock);
	~TCPSocket();

	// Allows for custom implicit conversion to type int
	operator const int() const;
};
