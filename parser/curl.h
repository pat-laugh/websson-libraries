//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <sstream>

class Curl
{
private:
	void* curl;
	char errorBuffer[256];
public:
	Curl();
	~Curl();

	void readWebDocument(std::stringstream& ss, const char* fileName);
	void readWebDocument(std::stringstream& ss, std::string&& fileName);
	void readWebDocument(std::stringstream& ss, const std::string& fileName);

	std::stringstream readWebDocument(const char* fileName);
	std::stringstream readWebDocument(std::string&& fileName);
	std::stringstream readWebDocument(const std::string& fileName);
};