//MIT License
//Copyright 2017-2018 Patrick Laughrea
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


#ifndef COMPILE_WEBSS
#define checkEscapedChar(It, Sb, StrL) checkEscapedChar(It, Sb)
#else
#define checkEscapedChar(It, Sb, StrL) checkEscapedChar(It, Sb, StrL)
#endif

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb, StringList*& stringList);
static inline void putChar(SmartIterator& it, StringBuilder& sb);
static bool isEnd(SmartIterator& it, function<bool()> endCondition);
static bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition = []() { return false; });
static void checkStringSubstitution(Parser& parser, StringBuilder& sb, StringList*& stringList);
static void pushStringList(StringList*& stringList, StringBuilder& sb, StringItem item);


#define PatternCheckCharEscape \
if (*it == CHAR_ESCAPE) { checkEscapedChar(it, sb, stringList); continue; }

#define PatternCheckCharSubstitution \
if (*it == CHAR_SUBSTITUTION) { checkStringSubstitution(parser, sb, stringList); continue; }

#define _PatternReturnStringList \
if (stringList == nullptr) \
	return sb.str(); \
stringList->push(sb.str());

#define PatternReturnStringListConcat \
_PatternReturnStringList \
return stringList->concat();

#define PatternReturnStringList \
_PatternReturnStringList \
return Webss(stringList, WebssType::STRING_LIST);

#ifndef COMPILE_WEBSS

#define _PatternReturnStringListPrint \
if (stringList == nullptr) \
	return sb.str(); \
stringList->push(sb.str());

#define PatternReturnStringListPrint \
_PatternReturnStringList \
return Webss(stringList, WebssType::STRING_LIST);

#else

#define _PatternReturnStringListPrint \
if (stringList == nullptr) \
	return Webss(sb.str(), WebssType::PRINT_STRING); \
stringList->push(sb.str());

#define PatternReturnStringListPrint \
_PatternReturnStringListPrint \
return Webss(stringList, WebssType::PRINT_STRING_LIST);

#endif

string webss::parseStickyLineString(Parser& parser)
{
	auto& it = parser.getItSafe();
	StringBuilder sb;
	StringList* stringList = nullptr;
	if (parser.multilineContainer)
		while (it && !isJunk(*it))
		{
			PatternCheckCharEscape;
			PatternCheckCharSubstitution;
			putChar(it, sb);
		}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar(), endChar = parser.con.getEndChar();
		while (it && !isJunk(*it))
		{
			PatternCheckCharEscape;
			PatternCheckCharSubstitution;
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
		PatternCheckCharEscape;
		putChar(it, sb);
	}
#ifdef COMPILE_WEBSS
	PatternReturnStringListConcat;
#else
	return sb;
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
			PatternCheckCharEscape;
			PatternCheckCharSubstitution;
			putChar(it, sb);
		}
	else
	{
		int countStartEnd = 1;
		char startChar = parser.con.getStartChar();
		while (hasNextChar(it, sb, [&]() { return *it == CHAR_SEPARATOR || (parser.con.isEnd(*it) && --countStartEnd == 0); }))
		{
			PatternCheckCharEscape;
			PatternCheckCharSubstitution;
			if (*it == startChar)
				++countStartEnd;
			putChar(it, sb);
		}
	}
	PatternReturnStringListPrint;
}

static Webss parseMultilineStringRegularMultiline(Parser& parser)
{
	auto& it = parser.getIt();
	StringBuilder sb;
	StringList* stringList = nullptr;
	int countStartEnd = 0;
	bool addSpace = true;
lineStart:
	if (*it == CHAR_START_DICTIONARY)
		++countStartEnd;
loopStart:
	do
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(it, sb, stringList);
			if (hasNextChar(it, sb))
				goto loopStart;
			addSpace = false;
			break;
		}
		PatternCheckCharSubstitution;
		putChar(it, sb);
	} while (hasNextChar(it, sb));
	if (!it || !skipJunkToValid(++it))
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_MULTILINE_STRING));
	if (*it == CHAR_END_DICTIONARY && countStartEnd-- == 0)
	{
		++it;
		PatternReturnStringListPrint;
	}
	if (addSpace)
		sb += ' ';
	else
		addSpace = true;
	goto lineStart;
}

static Webss parseMultilineStringRegular(Parser& parser)
{
	auto& it = parser.getIt();
	Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, true);
	if (!skipJunk(it))
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_MULTILINE_STRING));
	if (*it == CHAR_END_DICTIONARY)
	{
		++it;
		return "";
	}
	if (parser.multilineContainer)
		return parseMultilineStringRegularMultiline(parser);
	
	StringBuilder sb;
	StringList* stringList = nullptr;
	int countStartEnd = 1;
	bool addSpace = true;
	function<bool()> endCondition = [&]() { return *it == CHAR_END_DICTIONARY && --countStartEnd == 0; };
loopStart:
	while (hasNextChar(it, sb, endCondition))
	{
innerLoop:
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(it, sb, stringList);
			if (hasNextChar(it, sb, endCondition))
				goto innerLoop;
			addSpace = false;
			break;
		}
		PatternCheckCharSubstitution;
		if (*it == CHAR_START_DICTIONARY)
			++countStartEnd;
		putChar(it, sb);
	};
	if (countStartEnd == 0)
	{
		++it;
		PatternReturnStringListPrint;
	}
	if (!it || !skipJunkToValid(++it))
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_MULTILINE_STRING));
	if (addSpace)
		sb += ' ';
	else
		addSpace = true;
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
			checkEscapedChar(it, sb, stringList);
			continue;
		case CHAR_SUBSTITUTION:
			checkStringSubstitution(parser, sb, stringList);
			continue;
		default:
			sb += *it;
		//control Ascii chars, except tab and newline, are ignored
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05:
		case 0x06: case 0x07: case 0x08: //case 0x09: case 0x0a: tab and newline
		case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		case 0x7f:
			++it;
			break;
		}
	}
	throw runtime_error(WEBSSON_EXCEPTION("cstring is not closed"));
}

static void checkEscapedChar(SmartIterator& it, StringBuilder& sb, StringList*& stringList)
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
		pushStringList(stringList, sb, StringType::FUNC_NEWLINE_FLUSH);
		break;
	case 'F':
		pushStringList(stringList, sb, StringType::FUNC_FLUSH);
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
			parser.getItSafe(); //make sure tag iterator is unsafe after parseValueOnly call
		}
		++it;
		break;
	}
	default:
		if (!isNameStart(*it))
			throw runtime_error(WEBSSON_EXCEPTION("invalid substitution"));
		const auto& ent = parser.getEntityManager().at(parseName(it));
		checkTypeSubstitution(ent.getContent());
		pushStringList(stringList, sb, Webss(ent));
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