//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "constants.h"
#include "utils.h"

namespace webss
{
	//returns whether or not c is a char between 'a' and 'z', between 'A' and 'Z' or non-ascii (unicode > 127)
	bool isNameStart(char c);

	//returns whether or not c is a char between 'a' and 'z', between 'A' and 'Z', a digit (between '0' and '9') or non-ascii (unicode > 127)
	bool isNameBody(char c);
	
	//returns true if c is '_' or '-', else false
	bool isNameSeparator(char c);

	//returns whether or not c is a digit or equal to '-' or '+'
	bool isNumberStart(char c);

	//'e', 'E', 'p' or 'P'
	bool isBaseSeparator(char c);

	//any ascii control char or space
	bool isJunk(char c);

	//junk except newline
	bool isLineJunk(char c);
};