#include "importManager.h"

//#define DISABLE_IMPORT
#ifndef DISABLE_IMPORT
#include "curl.h"
#endif

using namespace std;
using namespace webss;

stringstream ImportManager::importDocument(string link)
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	auto doc = docs.find(link);
	if (doc != docs.end())
	{
		stringstream ss;
		ss << doc->second.rdbuf();
		return ss;
	}

	auto content = Curl().readWebDocument(link);
	stringstream ss;
	ss << content.rdbuf();
	docs.insert({ link, move(ss) });
	return content;
#endif
}