#pragma once

#include <string>
#include <unordered_map>

namespace webss
{
	class ImportManager
	{
	private:
		static ImportManager instance;

		std::unordered_map<std::string, std::string> docs;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			return instance;
		}

		std::string importDocument(std::string link);
	};
}