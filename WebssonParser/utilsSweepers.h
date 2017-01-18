//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <functional>

#include "iterators.h"
#include "language.h"

namespace webss
{
	//skips junk and valid junk operators (line escape and comments)
	SmartIterator& skipJunk(SmartIterator& it);

	//skips line junk and valid junk operators (line escape and comments)
	SmartIterator& skipLineJunk(SmartIterator& it);

	//if it points to a valid line escape, then skips it and returns true, else returns false
	//throws an error if a valid line escape has an invalid operand (only line junk is allowed between '\\' and ('\n' or eof))
	bool checkLineEscape(SmartIterator& it);

	//if it points to a valid comment, then skips it and returns true, else returns false
	bool checkComment(SmartIterator& it);

	//skips junk and valid junk operators (line escape and comments) until valid non-junk char is met
	//throws an error if eof is met
	SmartIterator& skipJunkToValid(SmartIterator& it);

	//skips junk and valid junk operators (line escape and comments) until valid non-junk char is met and then tests the condition
	//throws an error if eof is met or if the condition fails
	SmartIterator& skipJunkToValidCondition(SmartIterator& it, std::function<bool()> condition);

	//calls skipLineJunk, then checks the char pointed by it
	//if it is not a separator nor an end of container, an error is thrown
	void cleanLine(SmartIterator& it, ConType con, char separator);

	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con, char separator);

	enum class TypeContainer { DICTIONARY, LIST, TUPLE, FUNCTION_HEAD, LINE_STRING, EQUAL, CSTRING, TEXT_DICTIONARY, TEXT_LIST, TEXT_TUPLE, TEXT_FUNCTION_HEAD };
	TypeContainer skipJunkToContainer(SmartIterator& it);
}