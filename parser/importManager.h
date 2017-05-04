//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "structures/webss.h"

namespace webss
{
	class ImportManager
	{
	private:
		std::unordered_map<std::string, std::unordered_map<std::string, Entity>> docs;
		std::unordered_map<std::string, std::thread::id> parsing;
		std::mutex mDocs, mParsing;

		ImportManager();
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		const std::unordered_map<std::string, Entity>& importDocument(const std::string& link);
	};
}