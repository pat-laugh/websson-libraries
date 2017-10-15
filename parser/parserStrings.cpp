//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserStrings.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "unicode.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

static const char* ERROR_MULTILINE_STRING = "multiline-string is not closed";

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb);
static inline void putChar(SmartIterator& it, StringBuilder& sb);
static bool isEnd(SmartIterator& it, function<bool()> endCondition);
static bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition = []() { return false; });
static bool checkStringExpand(Parser& parser, StringBuilder& sb);
static const string& expandString(Parser& parser);

string webss::parseStickyLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
	while (it && !isJunk(*it))
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(it, sb);
			continue;
		}
		else if (*it == CHAR_EXPAND && checkStringExpand(parser, sb))
			continue;
		putChar(it, sb);
	}
	return sb;

}

string webss::parseLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	skipLineJunk(it);
	StringBuilder sb;
	if (parser.multilineContainer)
	{
		while (hasNextChar(it, sb))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				continue;
			}
			else if (*it == CHAR_EXPAND && checkStringExpand(parser, sb))
				continue;
			putChar(it, sb);
		}
	}
	else
	{
		int countStartEnd = 1;
		while (hasNextChar(it, sb, [&]() { return *it == CHAR_SEPARATOR || (parser.con.isEnd(*it) && --countStartEnd == 0); }))
		{
			if (*it == CHAR_ESCAPE)
			{
				checkEscapedChar(it, sb);
				continue;
			}
			else if (*it == CHAR_EXPAND && checkStringExpand(parser, sb))
				continue;
			else if (parser.con.isStart(*it))
				++countStartEnd;
			putChar(it, sb);
		}
	}
	return sb;
}

static string parseMultilineStringRegular(Parser& parser)
{
	auto& it = parser.getIt();
	Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, true);
	StringBuilder sb;
	if (skipJunk(it) == CHAR_END_DICTIONARY)
	{
		++it;
		return "";
	}

	int countStartEnd = 1;
	bool addSpace = false;
	function<bool()> endCondition;
	if (parser.multilineContainer)
		endCondition = []() { return false; };
	else
		endCondition = [&]() { return *it == CHAR_END_DICTIONARY && --countStartEnd == 0; };
loopStart:
	while (hasNextChar(it, sb, endCondition))
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(it, sb);
			addSpace = false;
			continue;
		}
		else if (*it == CHAR_EXPAND && checkStringExpand(parser, sb))
		{
			addSpace = true;
			continue;
		}
		else if (*it == CHAR_START_DICTIONARY && !parser.multilineContainer)
			++countStartEnd;
		addSpace = true;
		putChar(it, sb);
	}
	if (!it)
		throw runtime_error(ERROR_MULTILINE_STRING);
	switch (countStartEnd)
	{
	default:
		skipJunkToValid(++it);
		break;
	case 1:
		if (*skipJunkToValid(++it) != CHAR_END_DICTIONARY)
			break;
	case 0:
		++it;
		return sb;
	}
	if (addSpace)
		sb += ' ';
	goto loopStart;
}

string webss::parseMultilineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	if (*it != CHAR_START_DICTIONARY)
		throw runtime_error(ERROR_UNEXPECTED);
	return parseMultilineStringRegular(parser);
}

string webss::parseCString(Parser& parser)
{
	auto& it = parser.getItSafe();
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
			if (checkStringExpand(parser, sb))
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

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb)
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

static inline void putChar(SmartIterator& it, StringBuilder& sb)
{
	sb += *it;
	++it;
}

static bool isEnd(SmartIterator& it, function<bool()> endCondition)
{
	return !it || *it == '\n' || endCondition();
}

static bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition)
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

static bool checkStringExpand(Parser& parser, StringBuilder& sb)
{
	auto& it = parser.getIt();
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	++it;
	sb += expandString(parser);
	return true;
}

static const string& expandString(Parser& parser)
{
	auto& it = parser.getIt();
	try
	{
		const Webss* value = &parser.getEnts().at(parseName(it)).getContent();
		while (it == CHAR_SCOPE && value->isNamespace() && it.peekGood() && isNameStart(it.peek()))
			value = &value->getNamespace().at(parseName(++it)).getContent();
		return value->getString();
	}
	catch (const exception&)
	{
		throw runtime_error("could not expand string entity");
	}
}