//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "unicode.hpp"
#include "utils/constants.hpp"
#include "utils/utils.hpp"
#include "utils/utilsWebss.hpp"
#include "errors.hpp"

using namespace std;
using namespace webss;

void checkEscapedChar(SmartIterator& it, StringBuilder& sb);

inline void putChar(SmartIterator& it, StringBuilder& sb)
{
	sb += *it;
	++it;
}

bool isEnd(SmartIterator& it, function<bool()> endCondition)
{
	return !it || *it == '\n' || endCondition();
}

bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition = []() { return false; })
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
		if (checkJunkOperators(it) && isEnd(it, endCondition))
			return false;
		while (spaces-- > 0)
			sb += ' ';
	}
	
	return true;
}

string Parser::parseLineString()
{
	skipLineJunk(it);
	StringBuilder sb;
	if (multilineContainer)
	{
		while (hasNextChar(it, sb))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				continue;
			}
			else if (*it == CHAR_EXPAND && checkStringEntity(sb))
				continue;
			putChar(it, sb);
		}
	}
	else
	{
		int countStartEnd = 1;
		while (hasNextChar(it, sb, [&]() { return *it == CHAR_SEPARATOR || (con.isEnd(*it) && --countStartEnd == 0); }))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				continue;
			}
			else if (*it == CHAR_EXPAND && checkStringEntity(sb))
				continue;
			else if (con.isStart(*it))
				++countStartEnd;
			putChar(it, sb);
		}
	}
	return sb;
}

string Parser::parseMultilineString()
{
	ContainerSwitcher switcher(*this, ConType::DICTIONARY, true);
	StringBuilder sb;
	if (*skipJunkToValid(it) == CLOSE_DICTIONARY)
		return "";

	int countStartEnd = 1;
	bool addSpace = false;
loopStart:
	if (multilineContainer)
	{
		while (hasNextChar(it, sb))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				addSpace = false;
				continue;
			}
			else if (*it == CHAR_EXPAND && checkStringEntity(sb))
			{
				addSpace = true;
				continue;
			}
			addSpace = true;
			putChar(it, sb);
		}
	}
	else
	{
		while (hasNextChar(it, sb, [&]() { return *it == CLOSE_DICTIONARY && --countStartEnd == 0; }))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				addSpace = false;
				continue;
			}
			else if (*it == OPEN_DICTIONARY)
				++countStartEnd;
			else if (*it == CHAR_EXPAND && checkStringEntity(sb))
			{
				addSpace = true;
				continue;
			}
			addSpace = true;
			putChar(it, sb);
		}
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
		return sb;
	}
	if (addSpace)
		sb += ' ';
	goto loopStart;
}

string Parser::parseCString()
{
	++it;
	StringBuilder sb;
	while (it)
	{
		switch (*it)
		{
		case '\n':
			throw runtime_error("can't have line break in cstring");
		case CHAR_CSTRING:
			++it;
			return sb;
		case CHAR_EXPAND:
			if (checkStringEntity(sb))
				continue;
			break;
		case CHAR_ESCAPE:
			checkEscapedChar(it, sb);
			continue;
		default:
			break;
		}
		if (!isControlAscii(*it))
			sb += *it;
		++it;
	}
	throw runtime_error("cstring is not closed");
}

void checkEscapedChar(SmartIterator& it, StringBuilder& sb)
{
	if (!++it)
		throw runtime_error(ERROR_EXPECTED);

	switch (*it)
	{
	case 'x': case 'u': case 'U':
		putEscapedHex(it, sb);
		return;

	case '0': sb += '\0'; break;
    case 'a': sb += '\a'; break;
    case 'b': sb += '\b'; break;
	case 'c': sb += 0x1b; break;
	case 'e': /* empty */ break;
    case 'f': sb += '\f'; break;
	case 'n': sb += '\n'; break;
	case 'r': sb += '\r'; break;
	case 's': sb += ' '; break;
	case 't': sb += '\t'; break;
    case 'v': sb += '\v'; break;
	default:
		if (!isSpecialAscii(*it))
			throw runtime_error("invalid char escape");
		sb += *it;
		break;
	}
	++it;
}

bool Parser::checkStringEntity(StringBuilder& sb)
{
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	++it;
	sb += parseStringEntity();
	return true;
}

const string& Parser::parseStringEntity()
{
	try
	{
		const Webss* value = &ents.at(parseName(it)).getContent();
		while (it == CHAR_SCOPE && it.peekGood() && isNameStart(it.peek()))
			value = &value->getNamespace().at(parseName(++it)).getContent();
		return value->getString();
	}
	catch (const exception&)
	{
		throw runtime_error("could not get string entity");
	}
}