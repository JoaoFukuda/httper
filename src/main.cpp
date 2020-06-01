#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv)
{
	int port = 27090;
	[[unlikely]] if (argc == 1) {
		std::cerr << "Error: No arguments provided. Use as follow: httper filename [port]" << std::endl;
		return 1;
	}
	if (argc == 3) {
		std::stringstream toint(argv[2]);
		toint >> port;
	}

	std::string filename(argv[1]);
	std::ifstream file(filename);
	[[unlikely]] if (!file) {
		std::cerr << "Error: No file " << filename << " found" << std::endl;
		return 2;
	}

	// Loads the file into memory by putting it in this string
	std::string filebuffer((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
	filebuffer += "\r\n\0";

	file.close();

	std::cout << "Sharing " << filename
			<< " (" << (filebuffer.length() - 3) << " bytes) on localhost:" << port
			<< "..." << std::endl;

	auto host_sock = socket(AF_INET, SOCK_STREAM, 0);
	[[unlikely]] if (host_sock == -1) {
		std::cerr << "Error: Socket could not be created" << std::endl;
		return 3;
	}

	{
		// Sets socket options for the system not to block the ports for
		// subsequent uses
		int opt(1);
		setsockopt(host_sock,
				SOL_SOCKET,
				SO_REUSEADDR | SO_REUSEPORT,
				&opt,
				sizeof(opt));
	}

	sockaddr_in host_info;
	host_info.sin_family = AF_INET;
	host_info.sin_port = htons(port);
	host_info.sin_addr.s_addr = INADDR_ANY;

	if (bind(host_sock, reinterpret_cast<sockaddr*>(&host_info), sizeof(host_info)) == -1) {
		std::cerr << "Error: Could not bind socket to port " << port << std::endl;
		return 4;
	}

	[[unlikely]] if (listen(host_sock, 3) == -1) {
		std::cerr << "Error: Could not pot socket to listen" << std::endl;
		return 5;
	}

	while (true) {
		sockaddr_in client_info;
		auto client_size = sizeof(client_info);

		auto client_sock = accept(host_sock,
				reinterpret_cast<sockaddr*>(&client_info),
				reinterpret_cast<socklen_t*>(&client_size));

		[[unlikely]] if (client_sock == -1) {
			std::cerr << "Error: Client could not be accepted" << std::endl;
			continue;
		}

		std::cout << "Sending file" << std::endl;

		{
			std::stringstream buf;
			buf << "HTTP/1.1 200 OK\r\n"
				<< "Server: HTTPer\r\n"
				<< "Content-Length: " << ( filebuffer.size() - 3 ) << "\r\n"
				<< "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n"
				<< "Content-Type: text/plain\r\n\r\n\0";
			send(client_sock, buf.str().c_str(), buf.str().size(), 0);
		}

		send(client_sock, filebuffer.c_str(), filebuffer.size() - 2, 0);

		close(client_sock);

		std::cout << "File sent" << std::endl;
	}

	close(host_sock);
}

