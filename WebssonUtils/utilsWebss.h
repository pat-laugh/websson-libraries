//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "constants.h"
#include "utils.h"

namespace webss
{
	//returns whether or not c is a char between 'a' and 'z', between 'A' and 'Z', equal to '_' or non-ascii (unicode > 127)
	bool isNameStart(char c);

	//returns whether or not c is a char between 'a' and 'z', between 'A' and 'Z', equal to '_', a digit (between '0' and '9'), or non-ascii (unicode > 127)
	bool isNameBody(char c);

	//returns whether or not c is a digit or equal to '-' or '+'
	bool isNumberStart(char c);

	//'e', 'E', 'm' or 'M'
	bool isMagnitudeSeparator(char c);

	//any ascii control char or space
	bool isJunk(char c);

	//junk except newline
	bool isLineJunk(char c);

	//whether or not the char can be escaped
	//CLARIFICATION: special escapes that require more than 1 char, like \x and \u ARE included
	bool isEscapableChar(char c);

	//whether or not the char should always be escaped in a char
	//includes Ascii characers, '\\' and '?'
	bool isMustEscapeChar(char c);
};