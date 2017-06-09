//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <map>
#include <string>

#include "parser.hpp"
#include "utils/smartIterator.hpp"

namespace webss
{
	//an option string starts with ':' (line-string) or '=' (sticky line-string)
	//there is an error if parser's it is at end or if it doesn't point to an above char
	std::string parseOptionStringValue(Parser& parser);

	std::string parseOptionLine(Parser& parser, std::function<bool(char c)> endCondition);
}