//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <functional>

#include "iterators.h"
#include "language.h"

namespace webss
{
	//checks current char pointed by it; if junk advances it
	//repeats until it is at the end or on a non-junk char
	SmartIterator& skipJunk(SmartIterator& it);

	//checks current char pointed by it; if whitespace advances it
	//repeats until it is at the end or on a non-whitespace char (reminder: '\n' is not whitespace)
	//also skips over comment if there is one, stopping at the end of the line
	SmartIterator& skipWhitespace(SmartIterator& it);

	//checks current char pointed by it; if not '\n' advances it
	//repeats until it is at the end or on a '\n' char
	SmartIterator& skipLine(SmartIterator& it);

	SmartIterator& skipMultilineComment(SmartIterator& it);

	//skips whitespace until either the end or a new line char is reached, else throws an error
	//does not skip the new line char
	bool checkLineEscape(SmartIterator& it);

	bool checkComment(SmartIterator& it);

	//checks current char pointed by it; if junk advances it
	//repeats until it is at the end or on a non-junk char
	//if it is at the end, throws an error
	SmartIterator& skipJunkToValid(SmartIterator& it);

	//checks current char pointed by it; if junk advances it
	//repeats until it is at the end or on a non-junk char
	//if it is at the end or the condition fails, throws an error
	SmartIterator& skipJunkToValidCondition(SmartIterator& it, std::function<bool()> condition);

	//calls skipWhitespace, then checks the char pointed by it
	//if it is not a separator nor an end of container, an error is thrown
	void cleanLine(SmartIterator& it, ConType con, Language lang);
}