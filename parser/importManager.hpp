//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

#include "structures/webss.hpp"

namespace webss
{
	class ImportManager
	{
	private:
		std::unordered_map<std::string, std::pair<std::unordered_map<std::string, Entity>, std::vector<std::pair<std::string, Webss>>>> docs;
		std::unordered_map<std::string, std::thread::id> parsing;
		std::mutex mDocs, mParsing;

		ImportManager();
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		const std::pair<std::unordered_map<std::string, Entity>, std::vector<std::pair<std::string, Webss>>>& importDocument(const std::string& link);
	};
}