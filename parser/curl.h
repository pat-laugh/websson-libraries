#pragma once

#define CURL_STATICLIB

#include <curl/curl.h>
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

	CURL* curl;
	char errorBuffer[CURL_ERROR_SIZE];
public:
	Curl()
	{
		if (!(curl = curl_easy_init()))
			throw std::runtime_error("failed to init curl");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, function);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
	}

	~Curl()
	{
		curl_easy_cleanup(curl);
	}

	void readWebDocument(std::stringstream& ss, const char* fileName)
	{
		errorBuffer[0] = 0;
		curl_easy_setopt(curl, CURLOPT_URL, fileName);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);

		auto res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			throw std::runtime_error(curl_easy_strerror(res));
	}

	void readWebDocument(std::stringstream& ss, std::string&& fileName) { readWebDocument(ss, fileName.c_str()); }
	void readWebDocument(std::stringstream& ss, const std::string& fileName) { readWebDocument(ss, fileName.c_str()); }

	std::stringstream readWebDocument(const char* fileName)
	{
		std::stringstream ss;
		readWebDocument(ss, fileName);
		return ss;
	}
	std::stringstream readWebDocument(std::string&& fileName) { return readWebDocument(fileName.c_str()); }
	std::stringstream readWebDocument(const std::string& fileName) { return readWebDocument(fileName.c_str()); }
};