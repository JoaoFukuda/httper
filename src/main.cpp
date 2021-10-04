#include "network.hpp"
#include "filesystem.hpp"

#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

struct Settings {
	int port = 27090; // Default if no port is defined
	std::string filename;

	Settings(int argc, char* argv[])
	{
		if (argc == 1) {
			std::cerr << "Error: No arguments provided. Use as follow: httper "
			             "filename [port]"
			          << std::endl;
			throw(std::runtime_error("Settings()"));
		}
		else {
			filename = std::string(argv[1]);

			if (argc == 3) {
				// Converts std::string to int
				std::stringstream buffer(argv[2]);
				buffer >> port;
			}
		}
	}
};

class Server {
	TCPSocket m_sock;
	File m_file;
	int m_port;

	std::string build_http_header()
	{
		std::stringstream buf;

		buf << "HTTP/1.1 200 OK\r\n"
		    << "Server: HTTPer\r\n"
		    << "Content-Length: " << (m_file.buffer.size() - 3) << "\r\n"
		    << "Content-Disposition: attachment; filename=\"" << m_file.name
		    << "\"\r\n"
		    << "Content-Type: text/plain\r\n\r\n\0";

		return buf.str();
	}

	void send_file(int client_sock)
	{
		std::string header = build_http_header();

		send(client_sock, header.c_str(), header.size(), 0);
		send(client_sock, m_file.buffer.c_str(), m_file.buffer.size() - 2, 0);
	}

 public:
	Server(Settings settings) :
	    m_file(settings.filename), m_sock(), m_port(settings.port)
	{
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(m_port);
		hint.sin_addr.s_addr = INADDR_ANY;

		if (bind(m_sock, reinterpret_cast<sockaddr*>(&hint), sizeof(hint)) ==
		    -1) {
			std::cerr << "Error: Could not bind socket to port " << m_port
			          << std::endl;
			throw(std::runtime_error("bind()"));
		}

		if (listen(m_sock, 3) == -1) {
			std::cerr << "Error: Could not pot socket to listen" << std::endl;
			throw(std::runtime_error("listen()"));
		}
	}

	void run()
	{
		std::cout << "Sharing " << m_file.name << " ("
		          << (m_file.buffer.size() - 3)
		          << " bytes) on localhost:" << m_port << "..." << std::endl;

		while (true) {
			try {
				TCPSocket client_sock = accept(m_sock, nullptr, nullptr);

				std::cout << "Sending file" << std::endl;
				send_file(client_sock);
				std::cout << "File sent" << std::endl;
			}
			catch (...) {
				std::cerr << "Error: Client could not be accepted" << std::endl;
				continue;
			}
		}
	}
};

int main(int argc, char** argv)
{
	Settings args(argc, argv);
	Server server(args);
	server.run();
}
