//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "utils/smartIterator.hpp"
#include "utils/stringBuilder.hpp"

namespace webss
{
	//add an hex value's chars to str
	//REQUIREMENT: it must point to 'x', 'u' or 'U'
	void putEscapedHex(SmartIterator& it, StringBuilder& str);
}