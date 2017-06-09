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
	std::string parseOptionLine(Parser& parser, std::function<bool(char c)> endCondition);
}