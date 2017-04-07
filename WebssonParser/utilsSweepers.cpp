//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsSweepers.h"

#include <assert.h>

#include "errors.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

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
	case CHAR_USING_NAMESPACE: return Tag::USING_NAMESPACE;
	case CHAR_IMPORT: return Tag::IMPORT;
	case CHAR_OPTION: return Tag::OPTION;
	case CHAR_SELF: return Tag::SELF;
	case CHAR_SEPARATOR: return Tag::SEPARATOR;
	case CHAR_EXPLICIT_NAME: return Tag::EXPLICIT_NAME;
	case CHAR_SCOPE: return Tag::SCOPE;
	}
	if (isJunk(*it))
		return getTag(skipJunk(++it));
	if (isNameStart(*it))
		return Tag::NAME_START;
	if (isNumberStart(*it))
		return Tag::NUMBER_START;
	return Tag::UNKNOWN;
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