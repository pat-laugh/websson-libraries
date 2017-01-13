//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "iterators.h"

namespace webss
{
	//add an hex value's chars to str
	//REQUIREMENT: it must point to 'x', 'X', 'u' or 'U'
	void putEscapedHex(SmartIterator& it, std::string& str, char separator);
}