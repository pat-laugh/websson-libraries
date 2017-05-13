//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsSweepers.hpp"

#include <cassert>

#include "errors.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

void skipLineComment(SmartIterator& it);
void skipMultilineComment(SmartIterator& it);
void skipLineEscape(SmartIterator& it);

Tag getTagColon(SmartIterator& it)
{
	if (it.peekEnd() || it.peek() != CHAR_COLON)
		return Tag::LINE_STRING;
	switch (*skipJunkToValid(it.incTwo()))
	{
	case OPEN_DICTIONARY:
		return Tag::TEXT_DICTIONARY;
	case OPEN_LIST:
		return Tag::TEXT_LIST;
	case OPEN_TUPLE:
		return Tag::TEXT_TUPLE;
	case OPEN_TEMPLATE:
		return Tag::TEXT_TEMPLATE;
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

Tag webss::getTag(SmartIterator& it)
{
	if (!it)
		return Tag::NONE;
	switch (*it)
	{
	case CHAR_CSTRING: return Tag::C_STRING;
	case CHAR_COLON: return getTagColon(it);
	case CHAR_EQUAL: return Tag::EQUAL;
	case OPEN_DICTIONARY: return Tag::START_DICTIONARY;
	case OPEN_LIST: return Tag::START_LIST;
	case OPEN_TUPLE: return Tag::START_TUPLE;
	case OPEN_TEMPLATE: return Tag::START_TEMPLATE;
	case CLOSE_DICTIONARY: return Tag::END_DICTIONARY;
	case CLOSE_LIST: return Tag::END_LIST;
	case CLOSE_TUPLE: return Tag::END_TUPLE;
	case CLOSE_TEMPLATE: return Tag::END_TEMPLATE;
	case CHAR_ABSTRACT_ENTITY: return Tag::ENTITY_ABSTRACT;
	case CHAR_CONCRETE_ENTITY: return Tag::ENTITY_CONCRETE;
	case CHAR_USING_ONE: return Tag::USING_ONE;
	case CHAR_USING_ALL: return Tag::USING_ALL;
	case CHAR_IMPORT: return Tag::IMPORT;
	case CHAR_OPTION: return Tag::OPTION;
	case CHAR_SELF: return Tag::SELF;
	case CHAR_SEPARATOR: return Tag::SEPARATOR;
	case CHAR_EXPLICIT_NAME: return Tag::EXPLICIT_NAME;
	case CHAR_SCOPE: return Tag::SCOPE;

		//junk
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x20: case 0x7f:
		return getTag(skipJunk(++it));

		//junk operators
	case '/':
		return checkJunkOperators(it) ? getTag(it) : Tag::UNKNOWN;

		//name start
	case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		return Tag::NAME_START;
	default:
		return (unsigned char)*it > 127 ? Tag::NAME_START : Tag::UNKNOWN;

		//number start
	case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
	case '-': case '+':
		return Tag::NUMBER_START;
	}
}

SmartIterator& webss::skipJunkToTag(SmartIterator& it, Tag tag)
{
	if (getTag(it) == tag)
		return it;
	throw runtime_error(it ? ERROR_UNEXPECTED : ERROR_EXPECTED);
}

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	while (it)
		if (isJunk(*it))
			++it;
		else if (!checkJunkOperators(it))
			break;
	return it;
}

SmartIterator& webss::skipLineJunk(SmartIterator& it)
{
	while (it)
		if (isLineJunk(*it))
			++it;
		else
		{
			checkJunkOperators(it);
			break;
		}
	return it;
}

void skipLineComment(SmartIterator& it)
{
	while (it && *it != '\n')
		++it;
}

void skipMultilineComment(SmartIterator& it)
{
	if (!it)
		throw runtime_error("multiline comment is not closed");
	
	int num = 1;
	for (char c = *it; ++it; c = *it)
		if (c == '*')
		{
			if (*it != '/')
				continue;
			if (--num == 0)
			{
				skipLineJunk(++it);
				return;
			}
			if (!++it)
				break;
		}
		else if (c == '/' && *it == '*')
		{
			++num;
			if (!++it)
				break;
		}
	
	throw runtime_error("multiline comment is not closed");
}

void skipLineEscape(SmartIterator& it)
{
loopStart:
	if (!it)
		return;
	if (*it == '\n')
	{
		skipLineJunk(++it);
		return;
	}
	if (isLineJunk(*it)) //only chars allowed between /~ and \n are line-junk
	{
		++it;
		goto loopStart;
	}
	throw runtime_error(ERROR_UNEXPECTED);
}

bool webss::checkJunkOperators(SmartIterator& it)
{
	if (*it != CHAR_COMMENT || it.peekEnd())
		return false;

	switch (it.peek())
	{
	case CHAR_COMMENT:
		skipLineComment(it.incTwo());
		break;
	case '*':
		skipMultilineComment(it.incTwo());
		break;
	case '~':
		skipLineEscape(it.incTwo());
		break;
	default:
		return false;
	}
	return true;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(ERROR_EXPECTED);
	return it;
}

bool webss::isLineEnd(char c, ConType con)
{
	return c == '\n' || c == CHAR_SEPARATOR || con.isEnd(c);
}

bool checkEndOfLine(SmartIterator& it)
{
	if (!it)
		return true;
	if (*it != '\n')
		return false;
	++it;
	return true;
}

bool webss::checkLineEmpty(SmartIterator& it)
{
	if (checkEndOfLine(it))
		return true;
	if (isJunk(*it))
		return checkEndOfLine(skipLineJunk(++it));
	return checkJunkOperators(it) && checkEndOfLine(it);
}

string webss::parseName(SmartIterator& it)
{
	assert(it && isNameStart(*it));
	string name;
	name += *it;
	while (++it)
		if (isNameBody(*it))
			name += *it;
		else if (isNameSeparator(*it) && it.peekGood() && isNameBody(it.peek()))
		{
			name += *it;
			name += *++it;
		}
		else
			break;
	return name;
}