#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

namespace webss
{
	class ImportManager
	{
	private:
		std::unordered_map<std::string, std::stringstream> docs;

		ImportManager() {}
	public:
		static ImportManager& getInstance()
		{
			static ImportManager instance;
			return instance;
		}

		std::stringstream importDocument(std::string link);
	};
}