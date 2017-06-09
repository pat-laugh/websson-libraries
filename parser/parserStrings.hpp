//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "parser.hpp"

namespace webss
{
	std::string parseStickyLineString(Parser & parser);
	std::string parseLineString(Parser& parser);
	std::string parseMultilineString(Parser& parser);
	std::string parseCString(Parser& parser);
}