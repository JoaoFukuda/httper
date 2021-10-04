#pragma once

#include <string>

struct File {
	std::string name;
	std::string buffer;

	File(std::string filename);
};
