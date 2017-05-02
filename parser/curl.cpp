//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "curl.h"

#define CURL_STATICLIB

#include <curl/curl.h>

using namespace std;

Curl::Curl()
{
	if (!(curl = curl_easy_init()))
		throw std::runtime_error("failed to init curl");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, function);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
}

Curl::~Curl()
{
	curl_easy_cleanup(curl);
}

void Curl::readWebDocument(std::stringstream& ss, const char* fileName)
{
	errorBuffer[0] = 0;
	curl_easy_setopt(curl, CURLOPT_URL, fileName);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);

	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		throw std::runtime_error(curl_easy_strerror(res));
}

void Curl::readWebDocument(std::stringstream& ss, std::string&& fileName) { readWebDocument(ss, fileName.c_str()); }
void Curl::readWebDocument(std::stringstream& ss, const std::string& fileName) { readWebDocument(ss, fileName.c_str()); }

std::stringstream Curl::readWebDocument(const char* fileName)
{
	std::stringstream ss;
	readWebDocument(ss, fileName);
	return ss;
}
std::stringstream Curl::readWebDocument(std::string&& fileName) { return readWebDocument(fileName.c_str()); }
std::stringstream Curl::readWebDocument(const std::string& fileName) { return readWebDocument(fileName.c_str()); }