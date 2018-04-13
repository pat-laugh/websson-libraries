//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "parser.hpp"

namespace webss
{
	std::string parseStickyLineString(Parser& parser);
	std::string parseStickyLineStringOption(Parser& parser);
	Webss parseLineString(Parser& parser);
	Webss parseMultilineString(Parser& parser);
	Webss parseCString(Parser& parser);
}