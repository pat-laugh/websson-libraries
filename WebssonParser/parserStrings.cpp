//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "unicode.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utils.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

inline void putChar(SmartIterator& it, StringBuilder& s)
{
	s += *it;
	++it;
}

bool isEnd(SmartIterator& it, function<bool()> endCondition)
{
	return !it || *it == '\n' || endCondition();
}

bool hasNextChar(SmartIterator& it, StringBuilder& line, function<bool()> endCondition = []() {})
{
	if (isEnd(it, endCondition))
		return false;

	if (isLineJunk(*it))
	{
		int spaces = 0;
		do
		{
			if (*it == ' ')
				++spaces;
			if (isEnd(++it, endCondition))
				return false;
		} while (isLineJunk(*it));
		if (*it == CHAR_COMMENT && checkComment(it) && isEnd(it, endCondition))
			return false;
		else if (*it == CHAR_ESCAPE && checkLineEscape(it) && isEnd(it, endCondition))
			return false;
		while (spaces-- > 0)
			line += ' ';
	}
	
	return true;
}

string GlobalParser::Parser::parseLineString()
{
	skipLineJunk(it);

	StringBuilder line;
	while (hasNextChar(it, line, [&]() { return *it == CHAR_SEPARATOR || con.isEnd(*it); }))
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(line);
			continue;
		}
		else if (*it == CHAR_CONCRETE_ENTITY && checkStringEntity(line))
			continue;
		putChar(it, line);
	}
	return line;
}

string GlobalParser::Parser::parseMultilineString()
{
	StringBuilder text;
	if (*skipJunkToValid(++it) == CLOSE_DICTIONARY)
		return text;

	int countStartEnd = 1;
	bool addSpace = false;
loopStart:
	while (hasNextChar(it, text, [&]() { return *it == CLOSE_DICTIONARY && --countStartEnd == 0; }))
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(text);
			addSpace = false;
			continue;
		}
		else if (*it == OPEN_DICTIONARY)
			++countStartEnd;
		else if (*it == CHAR_CONCRETE_ENTITY && checkStringEntity(text))
		{
			addSpace = true;
			continue;
		}
		addSpace = true;
		putChar(it, text);
	}
	if (!it)
		throw runtime_error("multiline string is not closed");
	switch (countStartEnd)
	{
	default:
		skipJunkToValid(++it);
		break;
	case 1:
		if (*skipJunkToValid(++it) != CLOSE_DICTIONARY)
			break;
	case 0:
		++it;
		return text;
	}
	if (addSpace)
		text += ' ';
	goto loopStart;
}

string GlobalParser::Parser::parseCString()
{
	++it;
	StringBuilder cstr;
	while (it)
	{
		switch (*it)
		{
		case '\n':
			throw runtime_error("can't have line break in cstring");
		case CHAR_CSTRING:
			++it;
			return cstr;
		case CHAR_CONCRETE_ENTITY:
			if (checkStringEntity(cstr))
				continue;
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(cstr);
			continue;
		default:
			break;
		}
		if (!isControlAscii(*it))
			cstr += *it;
		++it;
	}
	throw runtime_error("cstring is not closed");
}

void GlobalParser::Parser::checkEscapedChar(StringBuilder& line)
{
	if (checkLineEscape(it))
		return;

	switch (*(++it)) //no need to check it is valid since if not valid, would've been a line escape
	{
	case 'x': case 'u': case 'U':
		putEscapedHex(it, line);
		return;

	case '0': line += '\0'; break;
    case 'a': line += '\a'; break;
    case 'b': line += '\b'; break;
	case 'e': /* empty */ break;
    case 'f': line += '\f'; break;
	case 'n': line += '\n'; break;
	case 'r': line += '\r'; break;
	case 's': line += ' '; break;
	case 't': line += '\t'; break;
    case 'v': line += '\v'; break;
	default:
		if (!isSpecialAscii(*it))
			throw runtime_error("invalid char escape");
		line += *it;
		break;
	}
	++it;
}

bool GlobalParser::Parser::checkStringEntity(StringBuilder& line)
{
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	++it;
	line += parseStringEntity();
	return true;
}

const string& GlobalParser::Parser::parseStringEntity()
{
	try
	{
		const Webss* value = &ents.at(parseName(it)).getContent();
		while (it == CHAR_SCOPE && it.peekGood() && isNameStart(it.peek()))
			value = &value->getNamespaceSafe().at(parseName(++it)).getContent();
		return value->getStringSafe();
	}
	catch (const exception&)
	{
		throw runtime_error("could not get string entity");
	}
}