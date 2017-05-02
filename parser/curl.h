#pragma once

#include <sstream>

class Curl
{
private:
	static size_t function(char* ptr, size_t size, size_t nmemb, std::stringstream* ss)
	{
		auto length = size * nmemb;
		auto end = (char*)ptr + length;
		while (ptr < end)
			*ss << *ptr++;
		return length;
	}

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