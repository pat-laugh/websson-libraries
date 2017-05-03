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
		std::unordered_map<std::string, std::unordered_map<std::string, Entity>> docs;
		std::unordered_map<std::string, std::thread::id> parsing;
		std::mutex mDocs, mParsing;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		const std::unordered_map<std::string, Entity>& importDocument(const std::string& link)
		{
#ifdef DISABLE_IMPORT
			throw runtime_error("this parser cannot import documents");
#else
			{
				//check if doc was already parsed
				std::lock_guard<std::mutex> lockDocs(mDocs); //unlocks when out of scope
				auto itDocs = docs.find(link);
				if (itDocs != docs.end())
					return itDocs->second;

				//check if doc is being parsed
				std::lock_guard<std::mutex> lockParsing(mParsing);
				auto itParsing = parsing.find(link);
				if (itParsing != parsing.end())
				{
					if (itParsing->second != std::this_thread::get_id())
						goto checkAgainLater; //unlocks both locks
					
					parsing.erase(link);
					throw std::runtime_error("can't have circular import");
				}
				parsing.insert({ link, std::this_thread::get_id() });
			}
			try
			{
				Parser parser(Curl().readWebDocument(link));
				parser.parseDocument();
				std::lock_guard<std::mutex> lockDocs(mDocs);
				std::lock_guard<std::mutex> lockParsing(mParsing);
				parsing.erase(link);
				std::unordered_map<std::string, Entity> ents;
				for (const auto& ent : parser.getEnts().getLocalEnts())
					ents.insert({ ent.getName(), ent });
				return docs.insert({ link, std::move(ents) }).first->second;
			}
			catch (const std::exception& e)
			{
				throw std::runtime_error(std::string("while parsing imported document \"") + link + "\", " + e.what());
			}
		checkAgainLater:
			std::this_thread::yield();
			return importDocument(link);
#endif
		}
	};
}