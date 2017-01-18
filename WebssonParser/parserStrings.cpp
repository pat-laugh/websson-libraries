//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "unicode.h"

using namespace std;
using namespace webss;

inline void putChar(It& it, StringBuilder& s)
{
	s += *it;
	++it;
}

bool isEnd(It& it, function<bool()> endCondition)
{
	return !it || *it == '\n' || endCondition();
}

bool hasNextChar(It& it, StringBuilder& line, function<bool()> endCondition = []() {})
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

void putEscapedChar(StringBuilder& str, char c)
{
	switch (c)
	{
	case '0': str += '\0'; break;
	case 'a': str += '\a'; break;
	case 'b': str += '\b'; break;
	case 'f': str += '\f'; break;
	case 'n': str += '\n'; break;
	case 'r': str += '\r'; break;
	case 't': str += '\t'; break;
	case 'v': str += '\v'; break;

	case 's': str += ' '; //no need for break
	case 'e': break; //empty

	default: //isSpecialAscii, else undefined behavior
		str += c;
	}
}

void Parser::checkEscapedChar(It& it, StringBuilder& line)
{
	if (checkLineEscape(it))
		return;
	if (!isEscapableChar(*(++it))) //no need to check it is valid since if not valid, would've been a line escape
		throw runtime_error("invalid char escape");

	switch (*it)
	{
	case 'x': case 'X': case 'u': case 'U':
		putEscapedHex(it, line, separator);
		break;
	default:
		putEscapedChar(line, *it);
		++it;
		break;
	}
}

bool Parser::checkStringEntity(It& it, StringBuilder& line)
{
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	line += CHAR_CONCRETE_ENTITY + parseStringEntity(++it);
	return true;
}

const string& Parser::parseStringEntity(It& it)
{
	try
	{
		const Webss* value = &ents.at(parseName(it)).getContent();
		while (it == CHAR_SCOPE && it.peekGood() && isNameStart(it.peek()))
			value = &value->getNamespace().at(parseName(++it)).getContent();
		return value->getString();
	}
	catch (exception e)
	{
		throw runtime_error("could not get string entity");
	}
}