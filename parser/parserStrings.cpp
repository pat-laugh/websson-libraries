//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserStrings.hpp"

#include "containerSwitcher.hpp"
#include "unicode.hpp"
#include "utils/constants.hpp"
#include "utils/utils.hpp"
#include "utils/utilsWebss.hpp"
#include "errors.hpp"

using namespace std;
using namespace webss;

const char ERROR_MULTILINE_STRING[] = "multiline-string is not closed";

void checkEscapedChar(SmartIterator& it, StringBuilder& sb);
inline void putChar(SmartIterator& it, StringBuilder& sb);
bool isEnd(SmartIterator& it, function<bool()> endCondition);
bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition = []() { return false; });
bool checkStringExpand(Parser& parser, StringBuilder& sb);
const string& expandString(Parser& parser);

string webss::parseStickyLineString(Parser& parser)
{
	auto& it = parser.getIt();
	StringBuilder sb;
	while (it && *it != ' ' && *it != '\n')
	{
		if (*it == CHAR_ESCAPE)
		{
			checkEscapedChar(it, sb);
			continue;
		}
		else if (*it == CHAR_EXPAND && checkStringExpand(parser, sb))
			continue;
		if (!isLineJunk(*it)) //ignore line-junk
			putChar(it, sb);
	}
	return sb;

}

string webss::parseLineString(Parser& parser)
{
	auto& it = parser.getIt();
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

struct MultilineStringOptions
{
	bool junkOperator, entity, indent, line, raw;
};

MultilineStringOptions checkMultilineStringOptions(SmartIterator& it)
{
	if (*it == OPEN_DICTIONARY)
		return MultilineStringOptions{ false, false, false, false, false };

	MultilineStringOptions options;
	StringBuilder sb;
	do
	{
		sb += *it;
		if (!++it)
			throw runtime_error(ERROR_MULTILINE_STRING);
	} while (*it != OPEN_DICTIONARY && *it != '~');
	auto optionString = sb.str();
	options.junkOperator = optionString.find("-c") != -1;
	options.entity = optionString.find("-e") != -1;
	options.indent = optionString.find("-i") != -1;
	options.line = optionString.find("-l") != -1;
	options.raw = optionString.find("-r") != -1;
	return options;
}

bool checkCRNL(SmartIterator& it)
{
	if (*it == '\r' && it.peekGood() && it.peek() == '\n')
	{
		++it;
		return true;
	}
	return false;
}

int countIndent(SmartIterator& it)
{
	int numIndent = 0;
	for (; it && isLineJunk(*it) && !checkCRNL(it); ++it)
		++numIndent;
	return numIndent;
}

//returns true if all junk skipped properly, else false
bool skipIndent(SmartIterator& it, int num)
{
	while (num-- > 0)
	{
		if (!it || !isLineJunk(*it) || checkCRNL(it))
			return false;
		++it;
	}
	return true;
}

string parseMultilineStringIndent(SmartIterator& it, bool opJunkOperators)
{
	StringBuilder sb;
	if (!it)
		return sb;

	//read past line the container start was on since indentation is not fetched from there
	if (*it != '\n')
		do
		{
			sb += *it;
			if (!++it)
				return sb;
		} while (*it != '\n');
	sb += '\n';
	++it;

	//ignore blank lines to get the indentation
	int numIndent;
	do
	{
		numIndent = countIndent(it);
		if (!it)
			return sb;
	} while (*it == '\n');

	do
	{
		while (*it == '\n')
		{
			sb += *it;
			if (!skipIndent(++it, numIndent))
			{
				if (!it || *it != '\n')
					return sb;
			}
			else if (!it)
				return sb;
		}
		if (isLineJunk(*it) && !opJunkOperators)
		{
			do
			{
				sb += *it;
				if (!++it)
					return sb;
			} while (isLineJunk(*it));
			checkJunkOperators(it);
			continue;
		}
		putChar(it, sb);
	} while (it);
	return sb;
}


string parseMultilineStringLineIndent(SmartIterator& it, bool opJunkOperators)
{
	StringBuilder sb;
	if (!it)
		throw runtime_error(ERROR_MULTILINE_STRING);

	//read past line the container start was on since indentation is not fetched from there
	if (*it != '\n')
		do
		{
			sb += *it;
			if (!++it)
				throw runtime_error(ERROR_MULTILINE_STRING);
		} while (*it != '\n');
	sb += '\n';
	++it;

	//ignore blank lines to get the indentation
	int numIndent;
	do
	{
		numIndent = countIndent(it);
		if (!it)
			throw runtime_error(ERROR_MULTILINE_STRING);
	} while (*it == '\n');

	int countStartEnd = 1;
	do
	{
		while (*it == '\n')
		{
			sb += *it;
			if (!skipIndent(++it, numIndent))
			{
				if (!it || *it != '\n')
					throw runtime_error(ERROR_MULTILINE_STRING);
			}
			else if (!it)
				throw runtime_error(ERROR_MULTILINE_STRING);
		}
		if (isLineJunk(*it) && !opJunkOperators)
		{
			do
			{
				sb += *it;
				if (!++it)
					throw runtime_error(ERROR_MULTILINE_STRING);
			} while (isLineJunk(*it));
			checkJunkOperators(it);
			continue;
		}
		else if (*it == OPEN_DICTIONARY)
			++countStartEnd;
		else if (*it == CLOSE_DICTIONARY && --countStartEnd == 0)
		{
			++it;
			return sb;
		}
		putChar(it, sb);
	} while (it);
	throw runtime_error(ERROR_MULTILINE_STRING);
}

string parseMultilineStringNoIndent(SmartIterator& it, bool opJunkOperators)
{
	StringBuilder sb;
	while (true)
	{
		if (!it)
			throw runtime_error(ERROR_MULTILINE_STRING);
		while (*it == '\n')
		{
			sb += *it;
			while (++it && isLineJunk(*it))
				sb += *it;
			if (!it)
				throw runtime_error(ERROR_MULTILINE_STRING);
			if (!opJunkOperators)
			{
				checkJunkOperators(it);
				if (!it)
					throw runtime_error(ERROR_MULTILINE_STRING);
			}
			if (*it == CLOSE_DICTIONARY)
			{
				++it;
				return sb;
			}
		}
		if (isLineJunk(*it) && !opJunkOperators)
		{
			do
			{
				sb += *it;
				if (!++it)
					throw runtime_error(ERROR_MULTILINE_STRING);
			} while (isLineJunk(*it));
			checkJunkOperators(it);
			continue;
		}
		putChar(it, sb);
	}
}

string parseMultilineStringLineNoIndent(SmartIterator& it, bool opJunkOperators)
{
	int countStartEnd = 1;
	StringBuilder sb;
	while (true)
	{
		if (!it)
			throw runtime_error(ERROR_MULTILINE_STRING);
		if (isLineJunk(*it) && !opJunkOperators)
		{
			do
			{
				sb += *it;
				if (!++it)
					throw runtime_error(ERROR_MULTILINE_STRING);
			} while (isLineJunk(*it));
			checkJunkOperators(it);
			continue;
		}
		else if (*it == OPEN_DICTIONARY)
			++countStartEnd;
		else if (*it == CLOSE_DICTIONARY && --countStartEnd == 0)
		{
			++it;
			return sb;
		}
		putChar(it, sb);
	}
}

string parseMultilineStringRegular(Parser& parser)
{
	auto& it = parser.getIt();
	Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, true);
	StringBuilder sb;
	if (skipJunk(it) == CLOSE_DICTIONARY)
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
		endCondition = [&]() { return *it == CLOSE_DICTIONARY && --countStartEnd == 0; };
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
		else if (*it == OPEN_DICTIONARY && !parser.multilineContainer)
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

bool isEndSpecial(SmartIterator& it)
{
	return !it || *it == '\n';
}

bool hasNextCharSpecial(SmartIterator& it, StringBuilder& sb)
{
	if (isEndSpecial(it))
		return false;

	if (isLineJunk(*it))
	{
		int spaces = 0;
		do
		{
			if (*it == ' ')
				++spaces;
			if (isEndSpecial(++it))
				return false;
		} while (isLineJunk(*it));
		if (isEndSpecial(it))
			return false;
		while (spaces-- > 0)
			sb += ' ';
	}

	return true;
}

string webss::parseMultilineString(Parser& parser)
{
	auto& it = parser.getIt();
	if (*it == OPEN_DICTIONARY)
		return parseMultilineStringRegular(parser);
	auto options = checkMultilineStringOptions(it);

	string content;
	if (*it == '~') //tab container automatically has the equivalent of multiline and indent
		content = parseMultilineStringIndent(++it, options.junkOperator);
	else
	{
		Parser::ContainerSwitcher switcher(parser, ConType::DICTIONARY, true);
		if (parser.multilineContainer)
		{
			if (options.indent)
			{
				content = parseMultilineStringIndent(it, options.junkOperator);
				if (it != CLOSE_DICTIONARY)
					throw runtime_error(ERROR_MULTILINE_STRING);
				++it;
			}
			else
				content = parseMultilineStringNoIndent(it, options.junkOperator);
		}
		else
		{
			if (options.indent)
				content = parseMultilineStringLineIndent(it, options.junkOperator);
			else
				content = parseMultilineStringLineNoIndent(it, options.junkOperator);
		}
	}

	SmartIterator itTemp = move(it);
	it = SmartIterator(move(content));
	StringBuilder sb;
	if (options.line)
	{
		while (it)
		{
			if (*it == CHAR_ESCAPE && !options.raw)
			{
				checkEscapedChar(it, sb);
				continue;
			}
			else if (*it == CHAR_EXPAND && !options.entity && checkStringExpand(parser, sb))
				continue;
			checkCRNL(it);
			putChar(it, sb);
		}
		it = move(itTemp);
		return sb;
	}

	bool addSpace = false;
loopStart:
	while (it && isJunk(*it))
		++it;
	if (!it)
	{
		it = move(itTemp);
		return sb;
	}
	while (hasNextCharSpecial(it, sb))
	{
		if (*it == CHAR_ESCAPE && !options.raw)
		{
			checkEscapedChar(it, sb);
			addSpace = false;
			continue;
		}
		else if (*it == CHAR_EXPAND && !options.entity && checkStringExpand(parser, sb))
		{
			addSpace = true;
			continue;
		}
		addSpace = true;
		putChar(it, sb);
	}
	if (!it)
	{
		it = move(itTemp);
		return sb;
	}
	if (addSpace)
		sb += ' ';
	goto loopStart;
}

string webss::parseCString(Parser& parser)
{
	auto& it = parser.getIt();
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

inline void putChar(SmartIterator& it, StringBuilder& sb)
{
	sb += *it;
	++it;
}

bool isEnd(SmartIterator& it, function<bool()> endCondition)
{
	return !it || *it == '\n' || endCondition();
}

bool hasNextChar(SmartIterator& it, StringBuilder& sb, function<bool()> endCondition)
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

bool checkStringExpand(Parser& parser, StringBuilder& sb)
{
	auto& it = parser.getIt();
	if (it.peekEnd() || !isNameStart(it.peek()))
		return false;

	++it;
	sb += expandString(parser);
	return true;
}

const string& expandString(Parser& parser)
{
	auto& it = parser.getIt();
	try
	{
		const Webss* value = &parser.getEnts().at(parseName(it)).getContent();
		while (it == CHAR_SCOPE && it.peekGood() && isNameStart(it.peek()))
			value = &value->getNamespace().at(parseName(++it)).getContent();
		return value->getString();
	}
	catch (const exception&)
	{
		throw runtime_error("could not expand string entity");
	}
}