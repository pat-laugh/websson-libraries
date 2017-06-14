//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <set>

#include "utils/conType.hpp"
#include "utils/smartIterator.hpp"
#include "tagIterator.hpp"

namespace webss
{
	//skips junk and valid junk operators (line escape and comments)
	SmartIterator& skipJunk(SmartIterator& it);

	//skips line junk and valid junk operators (line escape and comments)
	SmartIterator& skipLineJunk(SmartIterator& it);

	//if it points to a valid junk operator, then skips it and returns true, else returns false
	//throws an error if there is a junk operator, but its operand is invalid
	bool checkJunkOperators(SmartIterator& it);

	//skips junk and valid junk operators (line escape and comments) until valid non-junk char is met
	//throws an error if eof is met
	SmartIterator& skipJunkToValid(SmartIterator& it);

	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con);

	//skips line-junk, then returns true if it is at the end or equals newline, else false
	bool checkLineEmpty(SmartIterator& it);
	
	//returns a name; it must point to a name-start char
	std::string parseName(SmartIterator& it);

	//skips the char currently pointed by it, skips junk, then parses a name
	std::string parseNameExplicit(TagIterator& tagit);
}