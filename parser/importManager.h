#pragma once

#include <string>
#include <unordered_map>

#include "entityManager.h"
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
		std::unordered_map<std::string, BasicEntityManager<Webss>> docs;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		const BasicEntityManager<Webss>& importDocument(const std::string& link)
		{
#ifdef DISABLE_IMPORT
			throw runtime_error("this parser cannot import documents");
#else
			auto doc = docs.find(link);
			if (doc == docs.end())
			{
				Parser parser(Curl().readWebDocument(link));
				parser.parseDocument();
				doc = docs.insert({ link, parser.getEnts() }).first;
			}

			return doc->second;
#endif
		}
	};
}