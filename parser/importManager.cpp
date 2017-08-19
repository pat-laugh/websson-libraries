//MIT License
//Copyright 2017 Patrick Laughrea
#include "importManager.hpp"

#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "curl.hpp"
#include "parser.hpp"
#include "structures/document.hpp"

using namespace std;
using namespace webss;

ImportManager::ImportManager() {}

const pair<unordered_map<string, Entity>, vector<pair<string, Webss>>>& ImportManager::importDocument(const string& link)
{
	{ //to scope the locks
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
		auto doc = parser.parseDocument();
		lock_guard<mutex> lockDocs(mDocs);
		lock_guard<mutex> lockParsing(mParsing);
		parsing.erase(link);
		unordered_map<string, Entity> ents;
		for (const auto& ent : parser.getEnts().getPublicEnts())
			ents.insert({ ent.getName(), ent });
		vector<pair<string, Webss>> keyValuesCopy;
		for (const auto& keyValue : doc.getBody().getOrderedKeyValues())
			if (keyValue.first == nullptr)
				keyValuesCopy.push_back({ "", *keyValue.second });
			else
				keyValuesCopy.push_back({ *keyValue.first, *keyValue.second });
		return docs.insert({ link, { move(ents), move(keyValuesCopy) } }).first->second; //insert returns pair<iterator to added pair, bool for insert success>
	}
	catch (const exception& e)
	{
		throw runtime_error(string("while parsing imported document \"") + link + "\", " + e.what());
	}
checkAgainLater:
	this_thread::yield();
	return importDocument(link);
}