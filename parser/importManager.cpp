#include "importManager.h"

//#define DISABLE_IMPORT
#ifndef DISABLE_IMPORT
#include "curl.h"
#endif

using namespace std;
using namespace webss;

string ImportManager::importDocument(string link)
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	auto doc = docs.find(link);
	if (doc != docs.end())
		return doc->second;
	
	auto content = Curl().readWebDocument(link).str();
	docs.insert({ link, content });
	return content;
#endif
}