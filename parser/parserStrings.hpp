//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

#include "parser.hpp"

namespace webss
{
	//only used to parse the line after an import statement
	std::string parseStickyLineString(Parser& parser);
	
	//only used to parse strings in options
	std::string parseStickyLineStringOption(Parser& parser);
	
	Webss parseLineString(Parser& parser);
	Webss parseMultilineString(Parser& parser);
	Webss parseCString(Parser& parser);
}