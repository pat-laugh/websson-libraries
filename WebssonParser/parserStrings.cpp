//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "unicode.h"

using namespace std;
using namespace webss;

void putEscapedChar(string& str, char c);
inline void putNonControlAscii(It& it, string& s)
{
	if (!isControlAscii(*it))
		s += *it;
	++it;
}

inline void trimLineSpace(string& line, string::size_type minLength)
{
	while (line.length() > minLength && line.back() == ' ')
		line.pop_back();
}

string Parser::parseLineString(It& it, ConType con)
{
	skipLineJunk(it);

	string line;
	string::size_type minLength = 0;
	while (it)
	{
		if (isLineEnd(*it, con, separator))
			break;

		switch (*it)
		{
		case CHAR_COMMENT:
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY:
			if (checkStringEntity(it, line))
			{
				minLength = line.length();
				continue;
			}
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, line);
			minLength = line.length();
			continue;
		default:
			break;
		}
		putNonControlAscii(it, line);
	};

	trimLineSpace(line, minLength);
	return line;
}

string Parser::parseMultilineString(It& it)
{
	string text;
	if (*skipJunkToValid(++it) == CLOSE_DICTIONARY)
		return text;

	int countStartEnd = 1;
loopStart:
	bool addSpace = parseMultilineStringLine(it, text, countStartEnd);
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

bool Parser::parseMultilineStringLine(It& it, string& text, int& countStartEnd)
{
	auto minLength = text.length();
	do
	{
		switch (*it)
		{
		case '\n':
			goto endLoop;
		case CLOSE_DICTIONARY:
			if (--countStartEnd == 0)
				goto endLoop;
			break;
		case CHAR_COMMENT:
			if (checkComment(it))
				continue;
			break;
		case CHAR_CONCRETE_ENTITY:
			if (checkStringEntity(it, text))
			{
				minLength = text.length() + 1; //allows 1 space to get in, else a space is added
				continue;
			}
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, text);
			minLength = text.length();
			continue;
		case OPEN_DICTIONARY:
			++countStartEnd;
			break;
		default:
			break;
		}
		putNonControlAscii(it, text);
	} while (it);
endLoop:
	trimLineSpace(text, minLength);
	return text.length() != minLength; //minLength only set when any of escapes SENT or entity is met
}

string Parser::parseCString(It& it)
{
	string cstr;
	while (it)
	{
		switch (*it)
		{
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
		case '\n':
			throw runtime_error("can't have line break in cstring");
		default:
			break;
		}
		putNonControlAscii(it, cstr);
	};
	throw runtime_error("cstring is not closed");
}

//adds the char corresponding to an escape; for serialization
//REQUIREMENT: the char must be an escapable char
void putEscapedChar(string& str, char c)
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

void Parser::checkEscapedChar(It& it, std::string& line)
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

bool Parser::checkStringEntity(It& it, string& line)
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