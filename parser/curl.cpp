//MIT License
//Copyright 2017 Patrick Laughrea
#include "curl.hpp"

namespace
{
#define CURL_STATICLIB
#include <curl/curl.h>
}

#include "errors.hpp"

using namespace std;

static size_t functionCurl(char* ptr, size_t size, size_t nmemb, stringstream* ss)
{
	auto length = size * nmemb;
	auto end = (char*)ptr + length;
	while (ptr < end)
		*ss << *ptr++;
	return length;
}

Curl::Curl()
{
	if (!(curl = curl_easy_init()))
		throw runtime_error(WEBSSON_EXCEPTION("failed to init curl"));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, functionCurl);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
}

Curl::~Curl()
{
	curl_easy_cleanup(curl);
}

void Curl::readWebDocument(stringstream& ss, const char* fileName)
{
	errorBuffer[0] = 0;
	curl_easy_setopt(curl, CURLOPT_URL, fileName);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);

	auto res = curl_easy_perform(curl);
	if (res != CURLE_OK)
		throw runtime_error(WEBSSON_EXCEPTION(curl_easy_strerror(res)));
}

void Curl::readWebDocument(stringstream& ss, string&& fileName) { readWebDocument(ss, fileName.c_str()); }
void Curl::readWebDocument(stringstream& ss, const string& fileName) { readWebDocument(ss, fileName.c_str()); }

stringstream Curl::readWebDocument(const char* fileName)
{
	stringstream ss;
	readWebDocument(ss, fileName);
	return ss;
}
stringstream Curl::readWebDocument(string&& fileName) { return readWebDocument(fileName.c_str()); }
stringstream Curl::readWebDocument(const string& fileName) { return readWebDocument(fileName.c_str()); }