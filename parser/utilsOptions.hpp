//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "parser.hpp"

namespace webss
{
	/* This returns a tokensized version of the option line
	"-" means an option name follows
	"." means a name follows
	":" means a value follows
	*/

	const std::string OPTION_NAME = "-", OPTION_SCOPE = ".", OPTION_VALUE = ":";

	std::vector<std::string> parseOptionLine(Parser& parser, std::function<bool(char c)> endCondition);

	std::vector<std::string> expandOptionString(Parser& parser, std::string s);
}