//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <set>

#include "utils/conType.hpp"
#include "tagIterator.hpp"
#include "various/smartIterator.hpp"

namespace webss
{
	//skips junk and valid junk operators (line escape and comments)
	various::SmartIterator& skipJunk(various::SmartIterator& it);

	//skips line junk and valid junk operators (line escape and comments)
	various::SmartIterator& skipLineJunk(various::SmartIterator& it);
	
	//skips all characters on a line and the next newline if there is one
	various::SmartIterator& skipLine(various::SmartIterator& it);

	//if it points to a valid junk operator, then skips it and returns true, else returns false
	//throws an error if there is a junk operator, but its operand is invalid
	bool checkJunkOperators(various::SmartIterator& it);

	//skips junk and valid junk operators (line escape and comments) until valid non-junk char is met
	//throws an error if eof is met
	various::SmartIterator& skipJunkToValid(various::SmartIterator& it);

	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con);

	//skips line-junk, then returns true if it is at the end or equals newline, else false
	bool checkLineEmpty(various::SmartIterator& it);
	
	//returns a name; it must point to a name-start char
	std::string parseName(various::SmartIterator& it);

	//makes sure that the next char is a name start without skipping through junk
	//does not invalidate the tag iterator
	void checkNameExplicit(TagIterator& tagit);

	//skips the char currently pointed by it, skips junk, then parses a name
	std::string parseNameExplicit(TagIterator& tagit);
}