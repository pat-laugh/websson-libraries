//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <set>

#include "WebssonUtils/typeContainers.h"
#include "WebssonUtils/iterators.h"

namespace webss
{
	enum class Tag {
		NONE, UNKNOWN,
		NAME_START, NUMBER_START, C_STRING, LINE_STRING, EQUAL,
		START_DICTIONARY, START_LIST, START_TUPLE, START_TEMPLATE,
		END_DICTIONARY, END_LIST, END_TUPLE, END_TEMPLATE,
		TEXT_DICTIONARY, TEXT_LIST, TEXT_TUPLE, TEXT_TEMPLATE,
		ENTITY_ABSTRACT, ENTITY_CONCRETE,
		USING_NAMESPACE, IMPORT, OPTION, SELF
	};

	//skips junk until it finds a Tag
	Tag getTag(SmartIterator& it);

	//skips junk until tag is found; if not, throws error
	SmartIterator& skipJunkToTag(SmartIterator& it, Tag tag);

	//skips junk until any tag in tags is found; if not, throws error
	SmartIterator& skipJunkToAnyTag(SmartIterator& it, std::set<Tag> tags);

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

	//calls skipLineJunk, then checks the char pointed by it
	//if it is not a separator nor an end of container, an error is thrown
	void cleanLine(SmartIterator& it, ConType con, char separator);

	//return true if c is end of line, separator or end of container, else false
	bool isLineEnd(char c, ConType con, char separator);

	//skips line-junk, then returns true if it is at the end or equals newline, else false
	bool checkLineEmpty(SmartIterator& it);
	
	std::string parseName(SmartIterator& it);
}