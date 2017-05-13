//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "importManager.hpp"

#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "parser.hpp"

//#define DISABLE_IMPORT
#ifndef DISABLE_IMPORT
#include "curl.hpp"
#endif

using namespace std;
using namespace webss;

ImportManager::ImportManager() {}

const unordered_map<string, Entity>& ImportManager::importDocument(const string& link)
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	{
		//check if doc was already parsed
		lock_guard<mutex> lockDocs(mDocs); //unlocks when out of scope
		auto itDocs = docs.find(link);
		if (itDocs != docs.end())
			return itDocs->second;

		//check if doc is being parsed
		lock_guard<mutex> lockParsing(mParsing);
		auto itParsing = parsing.find(link);
		if (itParsing != parsing.end())
		{
			if (itParsing->second != this_thread::get_id())
				goto checkAgainLater; //unlocks both locks

			parsing.erase(link);
			throw runtime_error("can't have circular import");
		}
		parsing.insert({ link, this_thread::get_id() });
	}
	try
	{
		Parser parser(Curl().readWebDocument(link));
		parser.parseDocument();
		lock_guard<mutex> lockDocs(mDocs);
		lock_guard<mutex> lockParsing(mParsing);
		parsing.erase(link);
		unordered_map<string, Entity> ents;
		for (const auto& ent : parser.getEnts().getLocalEnts())
			ents.insert({ ent.getName(), ent });
		return docs.insert({ link, move(ents) }).first->second;
	}
	catch (const exception& e)
	{
		throw runtime_error(string("while parsing imported document \"") + link + "\", " + e.what());
	}
checkAgainLater:
	this_thread::yield();
	return importDocument(link);
#endif
}