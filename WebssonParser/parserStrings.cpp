//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "unicode.h"

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

string Parser::parseLineString(It& it, ConType con)
{
	skipLineJunk(it);

	StringBuilder line;
	while (hasNextChar(it, line, [&]() { return *it == separator || con.isEnd(*it); }))
	{
		switch (*it)
		{
		case CHAR_COMMENT:
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY:
			if (checkStringEntity(it, line))
				continue;
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, line);
			continue;
		default:
			break;
		}
		putChar(it, line);
	}
	return line;
}

string Parser::parseMultilineString(It& it)
{
	StringBuilder text;
	if (*skipJunkToValid(++it) == CLOSE_DICTIONARY)
		return text;

	int countStartEnd = 1;
	bool addSpace = false;
loopStart:
	while (hasNextChar(it, text, [&]() { return *it == CLOSE_DICTIONARY && --countStartEnd == 0; }))
	{
		switch (*it)
		{
		case CHAR_COMMENT:
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY:
			if (checkStringEntity(it, text))
			{
				addSpace = true;
				continue;
			}
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, text);
			addSpace = false;
			continue;
		case OPEN_DICTIONARY:
			++countStartEnd;
			break;
		default:
			break;
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

string Parser::parseCString(It& it)
{
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
			if (checkStringEntity(it, cstr))
				continue;
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, cstr);
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

void Parser::checkEscapedChar(It& it, StringBuilder& line)
{
	if (!checkLineEscape(it))
		return;

	switch (*(++it)) //no need to check it is valid since if not valid, would've been a line escape
	{
	case 'x': case 'X': case 'u': case 'U':
		putEscapedHex(it, line, separator);
		return;

	case '0': line += '\0'; break;
	case 'e': /* empty */ break;
	case 'n': line += '\n'; break;
	case 'r': line += '\r'; break;
	case 's': line += ' '; break;
	case 't': line += '\t'; break;
	default:
		if (!isSpecialAscii(*it))
			throw runtime_error("invalid char escape");
		line += *it;
		break;
	}
	++it;
}

bool Parser::checkStringEntity(It& it, StringBuilder& line)
{
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	line += parseStringEntity(++it);
	return true;
}

const string& Parser::parseStringEntity(It& it)
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