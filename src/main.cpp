#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <asio.hpp>

using asio::ip::tcp;

int main(int argc, char** argv)
{
	if(argc != 2) return 0;

	std::string filename(argv[1]);

	std::ifstream file(filename, std::ios::ate);
	if(!file) return 0;

	auto filesize = file.tellg();
	file.seekg(0);

	std::cout << "Sharing " << filename << " (" << filesize << " bytes)..." << std::endl;

	std::string filebuffer;

	char buf[512];
	while(file.read(buf, 512).gcount() > 0) filebuffer.append(buf);

	file.close();

	try
	{
		asio::io_context io_context;
		tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 27090));
		tcp::socket socket(io_context);
		acceptor.accept(socket);
		asio::error_code ignored_error;

		{
			std::stringstream buf;
			buf << "HTTP/1.1 200 OK\r\n"
				<< "Server: HTTPer\r\n"
				<< "Content-Length: " << filesize << "\r\n"
				<< "Content-Disposition: attachment; filename=\"" << filename << "\"\r\n"
				<< "Content-Type: text/plain\r\n\r\n";
			asio::write(socket, asio::buffer(buf.str()), ignored_error);
		}
		asio::write(socket, asio::buffer(filebuffer), ignored_error);
	} catch(std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
}
