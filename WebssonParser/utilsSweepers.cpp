//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsSweepers.h"

#include <assert.h>

#include "errors.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

bool checkOperators(SmartIterator& it);
void skipLineComment(SmartIterator& it);
void skipMultilineComment(SmartIterator& it);

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
	if (!skipJunk(it))
		return Tag::NONE;
	switch (*it)
	{
	case CHAR_CSTRING: return Tag::C_STRING;
	case CHAR_COLON: return getTagColon(it);
	case CHAR_EQUAL: return Tag::EQUAL;
	case OPEN_DICTIONARY: return Tag::DICTIONARY;
	case OPEN_LIST: return Tag::LIST;
	case OPEN_TUPLE: return Tag::TUPLE;
	case OPEN_TEMPLATE: return Tag::TEMPLATE;
	case CHAR_ABSTRACT_ENTITY: return Tag::ENTITY_ABSTRACT;
	case CHAR_CONCRETE_ENTITY: return Tag::ENTITY_CONCRETE;
	case CHAR_USING_NAMESPACE: return Tag::USING_NAMESPACE;
	case CHAR_IMPORT: return Tag::IMPORT;
	case CHAR_OPTION: return Tag::OPTION;
	case CHAR_SELF: return Tag::SELF;
	}
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

SmartIterator& webss::skipJunkToAnyTag(SmartIterator& it, set<Tag> tags)
{
	if (tags.find(getTag(it)) != tags.end())
		return it;
	throw runtime_error(it ? ERROR_UNEXPECTED : ERROR_EXPECTED);
}

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	while (it)
		if (isJunk(*it))
			++it;
		else if (!checkOperators(it))
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
			checkOperators(it);
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

bool checkOperators(SmartIterator& it)
{
	if (*it == CHAR_ESCAPE)
		return checkLineEscape(it);
	else if (*it == CHAR_COMMENT)
		return checkComment(it);
	return false;
}

bool webss::checkLineEscape(SmartIterator& it)
{
	if (it.peekEnd())
	{
		++it;
		return true;
	}
	if (!isJunk(it.peek()))
		return false;
	
	while (++it && isLineJunk(*it))
		;
	if (!it)
		return true;
	if (*it != '\n')
		throw runtime_error(ERROR_UNEXPECTED);
	skipLineJunk(++it);
	return true;
}

bool webss::checkComment(SmartIterator& it)
{
	if (it.peekEnd())
		return false;
	
	char c = it.peek();
	if (c == CHAR_COMMENT)
		skipLineComment(it.incTwo());
	else if (c == '*')
		skipMultilineComment(it.incTwo());
	else
		return false;
	return true;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(ERROR_EXPECTED);
	return it;
}

SmartIterator& webss::skipJunkToValidCondition(SmartIterator& it, function<bool()> condition)
{
	skipJunkToValid(it);
	if (!condition())
		throw runtime_error(ERROR_UNEXPECTED);
	return it;
}

void webss::cleanLine(SmartIterator& it, ConType con, char separator)
{
	if (skipLineJunk(it) && !isLineEnd(*it, con, separator))
		throw runtime_error(ERROR_UNEXPECTED);
}

bool webss::isLineEnd(char c, ConType con, char separator)
{
	return c == '\n' || c == separator || con.isEnd(c);
}

#define CSCase(x) case_##x: case x
TypeContainer webss::skipJunkToContainer(SmartIterator& it)
{
	switch (*skipJunkToValid(it))
	{
	CSCase(OPEN_DICTIONARY):
		return TypeContainer::DICTIONARY;
	CSCase(OPEN_LIST):
		return TypeContainer::LIST;
	CSCase(OPEN_TUPLE):
		return TypeContainer::TUPLE;
	CSCase(OPEN_TEMPLATE):
		return TypeContainer::TEMPLATE_HEAD;
	CSCase(CHAR_COLON):
		if (it.peekEnd() || it.peek() != CHAR_COLON)
			return TypeContainer::LINE_STRING;
		switch (*skipJunkToValid(it.incTwo()))
		{
		case OPEN_DICTIONARY:
			return TypeContainer::MULTILINE_STRING;
		case OPEN_LIST:
			return TypeContainer::TEXT_LIST;
		case OPEN_TUPLE:
			return TypeContainer::TEXT_TUPLE;
		case OPEN_TEMPLATE:
			return TypeContainer::TEXT_TEMPLATE_HEAD;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	case CHAR_EQUAL:
		switch (*skipJunkToValid(++it))
		{
		case OPEN_DICTIONARY:
			goto case_OPEN_DICTIONARY;
		case OPEN_LIST:
			goto case_OPEN_LIST;
		case OPEN_TUPLE:
			goto case_OPEN_TUPLE;
		case OPEN_TEMPLATE:
			goto case_OPEN_TEMPLATE;
		case CHAR_COLON:
			goto case_CHAR_COLON;
		case CHAR_CSTRING:
			goto case_CHAR_CSTRING;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	CSCase(CHAR_CSTRING):
		return TypeContainer::CSTRING;
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}
#undef CSCase

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