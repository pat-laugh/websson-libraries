//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "various/smartIterator.hpp"
#include "various/stringBuilder.hpp"

namespace webss
{
	//add an hex value's chars to str
	//REQUIREMENT: it must point to 'x', 'u' or 'U'
	void putEscapedHex(various::SmartIterator& it, various::StringBuilder& str);
}