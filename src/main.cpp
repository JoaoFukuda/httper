#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

struct File {
	std::string name;
	std::string buffer;

	File(std::string filename) : name(filename)
	{
		std::ifstream file(name);
		if (!file) {
			std::cerr << "Error: No file " << filename << " found" << std::endl;
			throw(std::runtime_error("std::ifstream()"));
		}

		// Loads the file into memory by putting it in this string
		std::string filebuffer((std::istreambuf_iterator<char>(file)),
		                       std::istreambuf_iterator<char>());
		filebuffer += "\r\n";

		file.close();

		buffer = std::move(filebuffer);
	}
};

struct TCPSocket {
	int sock;

	TCPSocket(int a_sock) : sock(a_sock)
	{
		if (sock == -1) {
			std::cerr << "Error: Socket could not be created" << std::endl;
			throw(std::runtime_error("TCPSocket()"));
		}
	}

	~TCPSocket() { close(sock); }

	operator const int() const { return sock; }
};

struct Settings {
	int port = 27090;
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
				std::stringstream toint(argv[2]);
				toint >> port;
			}
		}
	}
};

int create_server_socket()
{
	int host_sock = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	setsockopt(host_sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	           sizeof(opt));

	return host_sock;
}

void setup_server(int host_sock, int port)
{
	sockaddr_in host_info;
	host_info.sin_family = AF_INET;
	host_info.sin_port = htons(port);
	host_info.sin_addr.s_addr = INADDR_ANY;

	if (bind(host_sock, reinterpret_cast<sockaddr*>(&host_info),
	         sizeof(host_info)) == -1) {
		std::cerr << "Error: Could not bind socket to port " << port << std::endl;
		throw(std::runtime_error("bind()"));
	}

	if (listen(host_sock, 3) == -1) {
		std::cerr << "Error: Could not pot socket to listen" << std::endl;
		throw(std::runtime_error("listen()"));
	}
}

int start_server(int port)
{
	int host_sock = create_server_socket();
	setup_server(host_sock, port);

	return host_sock;
}

std::string build_http_header(const File& file)
{
	std::stringstream buf;

	buf << "HTTP/1.1 200 OK\r\n"
	    << "Server: HTTPer\r\n"
	    << "Content-Length: " << (file.buffer.size() - 3) << "\r\n"
	    << "Content-Disposition: attachment; filename=\"" << file.name
	    << "\"\r\n"
	    << "Content-Type: text/plain\r\n\r\n\0";

	return buf.str();
}

void give_file_to_client(int client_sock, const File& file)
{
	std::string header = build_http_header(file);

	send(client_sock, header.c_str(), header.size(), 0);
	send(client_sock, file.buffer.c_str(), file.buffer.size() - 2, 0);
}

int main(int argc, char** argv)
{
	Settings args(argc, argv);

	File file(args.filename);
	TCPSocket host_sock = start_server(args.port);

	std::cout << "Sharing " << args.filename << " (" << (file.buffer.size() - 3)
	          << " bytes) on localhost:" << args.port << "..." << std::endl;

	while (true) {
		try {
			TCPSocket client_sock = accept(host_sock, nullptr, nullptr);

			std::cout << "Sending file" << std::endl;
			give_file_to_client(client_sock, file);
			std::cout << "File sent" << std::endl;
		}
		catch (...) {
			std::cerr << "Error: Client could not be accepted" << std::endl;
			continue;
		}
	}
}
