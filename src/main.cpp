#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

void parse_arguments(int argc, char** argv, int& port, std::string& filename)
{
	port = 27090;

	if (argc == 1) {
		std::cerr << "Error: No arguments provided. Use as follow: httper "
		             "filename [port]"
		          << std::endl;
		throw(std::runtime_error("args"));
	}
	else {
		filename = std::string(argv[1]);

		if (argc == 3) {
			std::stringstream toint(argv[2]);
			toint >> port;
		}
	}
}

std::string load_file(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "Error: No file " << filename << " found" << std::endl;
		throw(std::runtime_error("std::ifstream()"));
	}

	// Loads the file into memory by putting it in this string
	std::string filebuffer((std::istreambuf_iterator<char>(file)),
	                       std::istreambuf_iterator<char>());
	filebuffer += "\r\n";

	file.close();

	return filebuffer;
}

int create_server_socket()
{
	auto host_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (host_sock == -1) {
		std::cerr << "Error: Socket could not be created" << std::endl;
		throw(std::runtime_error("socket()"));
	}

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

std::string build_http_header(const std::string& filename,
                              const std::string& filebuffer)
{
	std::stringstream buf;

	buf << "HTTP/1.1 200 OK\r\n"
	    << "Server: HTTPer\r\n"
	    << "Content-Length: " << (filebuffer.size() - 3) << "\r\n"
	    << "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n"
	    << "Content-Type: text/plain\r\n\r\n\0";

	return buf.str();
}

void give_file_to_client(int client_sock, const std::string& filename,
                         const std::string& filebuffer)
{
	std::string header = build_http_header(filename, filebuffer);

	send(client_sock, header.c_str(), header.size(), 0);
	send(client_sock, filebuffer.c_str(), filebuffer.size() - 2, 0);
}

int main(int argc, char** argv)
{
	int port;
	std::string filename;
	parse_arguments(argc, argv, port, filename);

	std::string filebuffer = load_file(filename);
	int host_sock = start_server(port);

	std::cout << "Sharing " << filename << " (" << (filebuffer.length() - 3)
	          << " bytes) on localhost:" << port << "..." << std::endl;

	while (true) {
		auto client_sock = accept(host_sock, nullptr, nullptr);
		if (client_sock == -1) {
			std::cerr << "Error: Client could not be accepted" << std::endl;
			continue;
		}

		std::cout << "Sending file" << std::endl;
		give_file_to_client(client_sock, filename, filebuffer);
		std::cout << "File sent" << std::endl;

		close(client_sock);
	}

	close(host_sock);
}
