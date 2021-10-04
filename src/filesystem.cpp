#include "filesystem.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

File::File(std::string filename) : name(filename)
{
	std::ifstream file(name);
	if (!file) {
		std::cerr << "Error: No file " << name << " found" << std::endl;
		throw(std::runtime_error("std::ifstream()"));
	}

	// Loads file into memory
	std::string filebuffer((std::istreambuf_iterator<char>(file)),
	                       std::istreambuf_iterator<char>());
	filebuffer += "\r\n";

	file.close();

	buffer = std::move(filebuffer);
}
