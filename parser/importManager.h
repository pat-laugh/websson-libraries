#pragma once

#include <string>
#include <unordered_map>

namespace webss
{
	class ImportManager
	{
	private:
		std::unordered_map<std::string, std::string> docs;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		std::string importDocument(const std::string& link);
	};
}