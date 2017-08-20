//MIT License
//Copyright 2017 Patrick Laughrea
#include "importManager.hpp"

#include <fstream>
#include <mutex>
#include <regex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <boost/filesystem.hpp>

#include "curl.hpp"
#include "parser.hpp"
#include "structures/document.hpp"

using namespace std;
using namespace webss;

ImportManager::ImportManager() {}

enum class TypeFile { EMPTY, ERROR, NO_SLASHES, SLASHES, PROTOCOL_NO_SLASHES, PROTOCOL, FILE_NO_SLASHES, FILE };

const regex& getRegex()
{
	static bool done = false;
	static regex re;
	if (done)
		return re;
	/*
	string protocol = "(([-+.A-Za-z0-9]+):)?"
	string doubleSlashes = "(//)?"
	
	string host = "(" + hostname + "|" + hostnumber + ")";
	string hostname = "(" + domainlabel + "\\.)*" + toplabel;
	string domainlabel = "(" + alphadigit + "|" + 
	
	string host = "(([-.A-Za-z0-9]+):)"
	*/
	string s = "^(([-+.A-Za-z0-9]+):)?(//)?(.*)"; //like file:// and http://
	re = regex(s);
	return re;
}

bool protocolIsFile(const string& p)
{
	if (p.length() != 4)
		return false;
	if (p[0] != 'f' && p[0] != 'F')
		return false;
	if (p[1] != 'i' && p[1] != 'I')
		return false;
	if (p[2] != 'l' && p[2] != 'L')
		return false;
	if (p[3] != 'e' && p[3] != 'E')
		return false;
	return true;
}

TypeFile getFileType(const string& filename)
{
	if (filename.empty())
		return TypeFile::EMPTY;
	
	const auto& re = getRegex();
	smatch sm;
	regex_match(filename, sm, re);
	
	if (sm.str(4).empty())
		return TypeFile::ERROR;
	
	if (sm.str(2).empty())
		return sm.str(3).empty() ? TypeFile::NO_SLASHES : TypeFile::SLASHES;
	else if (protocolIsFile(sm.str(2)))
		return sm.str(3).empty() ? TypeFile::FILE_NO_SLASHES : TypeFile::FILE;
	else
		return sm.str(3).empty() ? TypeFile::PROTOCOL_NO_SLASHES : TypeFile::PROTOCOL;
}

const pair<unordered_map<string, Entity>, vector<pair<string, Webss>>>& ImportManager::importDocument(const string& link, const string& filename)
{
	bool isFileFilename;
	auto fileTypeFilename = getFileType(filename);
	assert(fileTypeFilename != TypeFile::ERROR);
	switch (fileTypeFilename)
	{
	case TypeFile::EMPTY:
	case TypeFile::NO_SLASHES: case TypeFile::SLASHES:
	case TypeFile::FILE: case TypeFile::FILE_NO_SLASHES:
		isFileFilename = true;
		break;
	default:
		isFileFilename = false;
		break;
	}
	
	bool isFileLink;
	auto fileTypeLink = getFileType(link);
	if (fileTypeLink == TypeFile::ERROR)
		throw runtime_error("could not resolve import \"" + link + "\"");
	switch (fileTypeLink)
	{
	case TypeFile::EMPTY:
	case TypeFile::NO_SLASHES: case TypeFile::SLASHES:
	case TypeFile::FILE: case TypeFile::FILE_NO_SLASHES:
		isFileLink = true;
		break;
	default:
		isFileLink = false;
		break;
	}
	
	bool readLocally;
	string fullLink;
	
	if (isFileFilename && isFileLink)
	{
		readLocally = true;
		string nameFilename, nameLink;
		if (fileTypeFilename == TypeFile::FILE)
			nameFilename = filename.substr(filename.find("://") + 3);
		else if (fileTypeFilename == TypeFile::FILE_NO_SLASHES)
			nameFilename = filename.substr(filename.find(":") + 1);
		else
			nameFilename = filename;
		if (fileTypeLink == TypeFile::FILE)
			nameLink = link.substr(link.find("://") + 3);
		else if (fileTypeLink == TypeFile::FILE_NO_SLASHES)
			nameLink = link.substr(link.find(":") + 1);
		else
			nameLink = link;
		try
		{
			auto pathFile = boost::filesystem::absolute(nameFilename);
			auto pathLink = boost::filesystem::absolute(nameLink, pathFile.parent_path());
			fullLink = boost::filesystem::canonical(pathLink).generic_string();
		}
		catch (boost::filesystem::filesystem_error&)
		{
			throw runtime_error("could not resolve import \"" + link + "\"");
		}
	}
	else
	{
		readLocally = false;
		fullLink = link;
	}
	
	{ //to scope the locks
		//check if doc was already parsed
		lock_guard<mutex> lockDocs(mDocs); //unlocks when out of scope
		auto itDocs = docs.find(fullLink);
		if (itDocs != docs.end())
			return itDocs->second;

		//check if doc is being parsed
		lock_guard<mutex> lockParsing(mParsing);
		auto itParsing = parsing.find(fullLink);
		if (itParsing != parsing.end())
		{
			if (itParsing->second != this_thread::get_id())
				goto checkAgainLater; //unlocks both locks

			parsing.erase(fullLink);
			throw runtime_error("can't have circular import");
		}
		parsing.insert({ fullLink, this_thread::get_id() });
	}
	
	try
	{
		Document doc;
		Parser parser;
		if (!readLocally)
			doc = parser.parseDocument(Curl().readWebDocument(fullLink), fullLink);
		else
		{
			ifstream fileIn(fullLink, ios::binary);
			if (fileIn.fail())
				throw runtime_error("failed to open file \"" + filename + "\"");
			doc = parser.parseDocument(fileIn, fullLink);
		}
			
		lock_guard<mutex> lockDocs(mDocs);
		lock_guard<mutex> lockParsing(mParsing);
		parsing.erase(fullLink);
		unordered_map<string, Entity> ents;
		for (const auto& ent : parser.getEnts().getPublicEnts())
			ents.insert({ ent.getName(), ent });
		vector<pair<string, Webss>> keyValuesCopy;
		for (const auto& keyValue : doc.getBody().getOrderedKeyValues())
			if (keyValue.first == nullptr)
				keyValuesCopy.push_back({ "", *keyValue.second });
			else
				keyValuesCopy.push_back({ *keyValue.first, *keyValue.second });
		return docs.insert({ fullLink, { move(ents), move(keyValuesCopy) } }).first->second; //insert returns pair<iterator to added pair, bool for insert success>
	}
	catch (const exception& e)
	{
		throw runtime_error("while parsing imported document \"" + link + "\", " + e.what());
	}
checkAgainLater:
	this_thread::yield();
	return importDocument(link, filename);
}