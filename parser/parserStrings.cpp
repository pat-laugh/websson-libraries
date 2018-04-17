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
static void checkStringSubstitution(Parser& parser, StringBuilder& sb);

#define CheckCharEscape do { \
if (*it == CHAR_ESCAPE) \
{ \
	checkEscapedChar(it, sb); \
	continue; \
} } while (false)

#define CheckCharSubstitution do { \
if (*it == CHAR_SUBSTITUTION) \
{ \
	checkStringSubstitution(parser, sb); \
	continue; \
} } while (false)

string webss::parseStickyLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
	if (parser.multilineContainer)
	{
		while (it && !isJunk(*it))
		{
			CheckCharEscape;
			CheckCharSubstitution;
			putChar(it, sb);
		}
	}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar(), endChar = parser.con.getEndChar();
		while (it && !isJunk(*it))
		{
			CheckCharEscape;
			CheckCharSubstitution;
			if (*it == CHAR_SEPARATOR)
				break;
			if (*it == endChar)
			{
				if (--countStartEnd == 0)
					break;
			}
			else if (*it == startChar)
				++countStartEnd;
			putChar(it, sb);
		}
	}
	return sb;
}

string webss::parseStickyLineStringOption(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
	while (it && !isJunk(*it))
	{
		CheckCharEscape;
		putChar(it, sb);
	}
	return sb;
}

Webss webss::parseLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	skipLineJunk(it);
	StringBuilder sb;
	StringList* stringList = nullptr;
	if (parser.multilineContainer)
		while (hasNextChar(it, sb))
		{
			CheckCharEscape;
			CheckCharSubstitution;
			putChar(it, sb);
		}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar();
		while (hasNextChar(it, sb, [&]() { return *it == CHAR_SEPARATOR || (parser.con.isEnd(*it) && --countStartEnd == 0); }))
		{
			CheckCharEscape;
			CheckCharSubstitution;
			if (*it == startChar)
				++countStartEnd;
			putChar(it, sb);
		}
	}
	if (stringList == nullptr)
		return sb;
	stringList->push_back(sb);
	return Webss(stringList, WebssType::STRING_LIST);
}

static Webss parseMultilineStringRegular(Parser& parser)
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
		else if (*it == CHAR_SUBSTITUTION)
		{
			checkStringSubstitution(parser, sb);
			addSpace = true;
			continue;
		}
		else if (*it == CHAR_START_DICTIONARY && !parser.multilineContainer)
			++countStartEnd;
		addSpace = true;
		putChar(it, sb);
	}
	if (!it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_MULTILINE_STRING));
	if (countStartEnd > 1)
		skipJunkToValid(++it);
	else if (countStartEnd == 0 || *skipJunkToValid(++it) == CHAR_END_DICTIONARY)
	{
		++it;
		return sb;
	}
	if (addSpace)
		sb += ' ';
	goto loopStart;
}

Webss webss::parseMultilineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	if (*it != CHAR_START_DICTIONARY)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	return parseMultilineStringRegular(parser);
}

Webss webss::parseCString(Parser& parser)
{
	auto& it = parser.getItSafe();
	++it;
	StringBuilder sb;
	while (it)
	{
		switch (*it)
		{
		case '\n':
			throw runtime_error(WEBSSON_EXCEPTION("can't have line break in cstring"));
		case CHAR_CSTRING:
			++it;
			return sb;
		case CHAR_ESCAPE:
			checkEscapedChar(it, sb);
			continue;
		case CHAR_SUBSTITUTION:
			checkStringSubstitution(parser, sb);
			continue;
		default:
			break;
		}
		if (!isControlAscii(*it) || *it == '\t')
			sb += *it;
		++it;
	}
	throw runtime_error(WEBSSON_EXCEPTION("cstring is not closed"));
}

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb)
{
	if (!++it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));

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
			throw runtime_error(WEBSSON_EXCEPTION("invalid char escape"));
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
		StringBuilder spacesAndTabs;
		do
		{
			if (*it == ' ' || *it == '\t')
				spacesAndTabs += *it;
			if (isEnd(++it, endCondition))
				return false;
		} while (isLineJunk(*it));
		if (checkJunkOperators(it) && isEnd(it, endCondition))
			return false;
		sb += spacesAndTabs;
	}

	return true;
}

static void substituteString(StringBuilder& sb, const Webss& webss)
{
	if (webss.isString())
		sb += webss.getString();
	else if (webss.isStringList())
		sb += webss.getStringList().concat();
	else
		throw runtime_error(WEBSSON_EXCEPTION("string substitution must evaluate to string"));
}

static void checkStringSubstitution(Parser& parser, StringBuilder& sb)
{
	auto& it = parser.getIt();
	if (!++it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
	switch (*it)
	{
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	case '_':
		; //do something...
		break;
	case '{':
	{
		Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, false);
		if (!skipJunk(it))
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
		if (*it != '}')
		{
			Webss webss = parser.parseValueOnly();
			if (!skipJunk(it) || *it != '}')
				throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
			substituteString(sb, webss);
		}
		++it;
		break;
	}
	default:
		if (!isNameStart(*it))
			throw runtime_error(WEBSSON_EXCEPTION("invalid substitution"));
		substituteString(sb, parser.getEntityManager().at(parseName(it)).getContent());
		break;
	}
}