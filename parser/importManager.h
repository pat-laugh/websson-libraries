#pragma once

#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "structures/webss.h"

//#define DISABLE_IMPORT
#ifndef DISABLE_IMPORT
#include "curl.h"
#endif

namespace webss
{
	template <class Parser>
	class ImportManager
	{
	private:
		std::unordered_map<std::string, std::vector<Entity>> docs;
		std::set<std::string> parsing;
		std::mutex mDocs, mParsing;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		const std::vector<Entity>& importDocument(const std::string& link)
		{
#ifdef DISABLE_IMPORT
			throw runtime_error("this parser cannot import documents");
#else
			{
				//check if doc was already parsed
				std::lock_guard<std::mutex> lockDocs(mDocs); //unlocks when out of scope
				auto doc = docs.find(link);
				if (doc != docs.end())
					return doc->second;

				//check if doc is being parsed
				std::lock_guard<std::mutex> lockParsing(mParsing);
				if (parsing.find(link) != parsing.end())
					goto checkAgainLater; //unlocks both locks
				parsing.insert(link);
			}
			try
			{
				Parser parser(Curl().readWebDocument(link));
				parser.parseDocument();
				std::lock_guard<std::mutex> lockDocs(mDocs);
				std::lock_guard<std::mutex> lockParsing(mParsing);
				parsing.erase(link);
				return docs.insert({ link, parser.getEnts().getLocalEnts() }).first->second;
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string("while parsing imported document \"") + link + "\", " + e.what());
			}
		checkAgainLater:
			this_thread::yield();
			return importDocument(link);
#endif
		}
	};
}