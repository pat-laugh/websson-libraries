//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "WebssonUtils/iterators.h"
#include "WebssonUtils/stringBuilder.h"

namespace webss
{
	//add an hex value's chars to str
	//REQUIREMENT: it must point to 'x', 'u' or 'U'
	void putEscapedHex(SmartIterator& it, StringBuilder& str);
}