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

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb
#ifdef COMPILE_WEBSS
	, StringList*& stringList
#endif
);
static inline void putChar(SmartIterator& it, StringBuilder& sb);
static bool isEnd(SmartIterator& it, function<bool()> endCondition);
static bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition = []() { return false; });
static void checkStringSubstitution(Parser& parser, StringBuilder& sb, StringList*& stringList);
static void pushStringList(StringList*& stringList, StringBuilder& sb, StringItem item);

#ifndef COMPILE_WEBSS
#define PatternCheckCharEscape(Cmd) do { \
if (*it == CHAR_ESCAPE) { checkEscapedChar(it, sb); Cmd; continue; } \
} while (false)
#else
#define PatternCheckCharEscape(Cmd) do { \
if (*it == CHAR_ESCAPE) { checkEscapedChar(it, sb, stringList); Cmd; continue; } \
} while (false)
#endif

#define PatternCheckCharSubstitution(Cmd) do { \
if (*it == CHAR_SUBSTITUTION) { checkStringSubstitution(parser, sb, stringList); Cmd; continue; } \
} while (false)

#define _PatternReturnStringList \
if (stringList == nullptr) \
	return sb.str(); \
stringList->push(sb.str());

#define PatternReturnStringListConcat do { \
_PatternReturnStringList \
return stringList->concat(); \
} while (false)

#define PatternReturnStringList do { \
_PatternReturnStringList \
return Webss(stringList, WebssType::STRING_LIST); \
} while (false)

string webss::parseStickyLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
	StringList* stringList = nullptr;
	if (parser.multilineContainer)
	{
		while (it && !isJunk(*it))
		{
			PatternCheckCharEscape();
			PatternCheckCharSubstitution();
			putChar(it, sb);
		}
	}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar(), endChar = parser.con.getEndChar();
		while (it && !isJunk(*it))
		{
			PatternCheckCharEscape();
			PatternCheckCharSubstitution();
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
	PatternReturnStringListConcat;
}

string webss::parseStickyLineStringOption(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
#ifdef COMPILE_WEBSS
	StringList* stringList = nullptr;
#endif
	while (it && !isJunk(*it))
	{
		PatternCheckCharEscape();
		putChar(it, sb);
	}
#ifndef COMPILE_WEBSS
	return sb;
#else
	PatternReturnStringListConcat;
#endif
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
			PatternCheckCharEscape();
			PatternCheckCharSubstitution();
			putChar(it, sb);
		}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar();
		while (hasNextChar(it, sb, [&]() { return *it == CHAR_SEPARATOR || (parser.con.isEnd(*it) && --countStartEnd == 0); }))
		{
			PatternCheckCharEscape();
			PatternCheckCharSubstitution();
			if (*it == startChar)
				++countStartEnd;
			putChar(it, sb);
		}
	}
	PatternReturnStringList;
}

static Webss parseMultilineStringRegular(Parser& parser)
{
	auto& it = parser.getIt();
	Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, true);
	if (skipJunk(it) == CHAR_END_DICTIONARY)
	{
		++it;
		return "";
	}
	
	StringBuilder sb;
	StringList* stringList = nullptr;

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
		PatternCheckCharEscape(addSpace = false);
		PatternCheckCharSubstitution(addSpace = true);
		if (*it == CHAR_START_DICTIONARY && !parser.multilineContainer)
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
		PatternReturnStringList;
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
	StringList* stringList = nullptr;
	while (it)
	{
		switch (*it)
		{
		case '\n':
			throw runtime_error(WEBSSON_EXCEPTION("can't have line break in cstring"));
		case CHAR_CSTRING:
			++it;
			PatternReturnStringList;
		case CHAR_ESCAPE:
			checkEscapedChar(it, sb
#ifdef COMPILE_WEBSS
			, stringList
#endif
			);
			continue;
		case CHAR_SUBSTITUTION:
			checkStringSubstitution(parser, sb, stringList);
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

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb
#ifdef COMPILE_WEBSS
, StringList*& stringList
#endif
)
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
#ifdef COMPILE_WEBSS
	case 'E':
		pushStringList(stringList, sb, StringType::FUNC_NEWLINE);
	case 'F':
		pushStringList(stringList, sb, StringType::FUNC_FLUSH);
		break;
	case 'K':
		pushStringList(stringList, sb, StringType::FUNC_CANCEL_FLUSH);
		break;
	case 'L':
		pushStringList(stringList, sb, StringType::FUNC_CANCEL_NEWLINE);
		break;
	case 'N':
		pushStringList(stringList, sb, StringType::FUNC_NEWLINE);
		break;
#endif
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

#ifndef COMPILE_WEBSS
static void checkTypeSubstitution(const Webss& webss)
{
	auto type = webss.getType();
	if (type != WebssType::PRIMITIVE_STRING && type != WebssType::STRING_LIST)
		throw runtime_error(WEBSSON_EXCEPTION("entity to substitute must be a string"));
}
#else
#define checkTypeSubstitution(X)
#endif

static void checkStringSubstitution(Parser& parser, StringBuilder& sb, StringList*& stringList)
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
			checkTypeSubstitution(webss);
			pushStringList(stringList, sb, move(webss));
		}
		++it;
		break;
	}
	default:
		if (!isNameStart(*it))
			throw runtime_error(WEBSSON_EXCEPTION("invalid substitution"));
		const auto& ent = parser.getEntityManager().at(parseName(it));
		checkTypeSubstitution(ent.getContent());
		pushStringList(stringList, sb, ent);
		break;
	}
}

static void pushStringList(StringList*& stringList, StringBuilder& sb, StringItem item)
{
	if (stringList == nullptr)
		stringList = new StringList();
	stringList->push(sb.str());
	sb.clear();
	stringList->push(move(item));
}