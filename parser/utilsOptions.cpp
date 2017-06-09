//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsOptions.hpp"

#include "parserStrings.hpp"
#include "utilsSweepers.hpp"
#include "utils/constants.hpp"
#include "utils/stringBuilder.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

const char ERROR_OPTION[] = "expected option";

string expandOptionString(Parser& parser, string s);

string parseColon(Parser& parser, bool expand)
{
	++parser.getIt();
	auto content = parseLineString(parser);
	return CHAR_COLON + (expand ? expandOptionString(parser, move(content)) : content);
}
string parseEqual(Parser& parser, bool expand)
{
	++parser.getIt();
	auto content = parseStickyLineString(parser);
	return CHAR_EQUAL + (expand ? expandOptionString(parser, move(content)) : content);
}

string webss::parseOptionStringValue(Parser& parser)
{
	auto& it = parser.getIt();
	if (!it)
		throw runtime_error("expected value");
	if (*it == CHAR_COLON)
	{
		++it;
		return parseLineString(parser);
	}
	else if (*it == CHAR_EQUAL)
	{
		++it;
		return parseStickyLineString(parser);
	}
	else
		throw runtime_error("expected value");
}

string parseOptionString(Parser& parser, bool expand)
{
	auto& it = parser.getIt();
	if (!it)
		throw runtime_error("expected value");
	if (*it == CHAR_COLON)
		return parseColon(parser, expand);
	else if (*it == CHAR_EQUAL)
		return parseEqual(parser, expand);
	else
		throw runtime_error("expected value");
}

void parseOptionScope(Parser& parser, StringBuilder& sb, bool expand)
{
	auto& it = parser.getIt();
	sb += CHAR_SCOPE;
	if (!++it || !isNameStart(*it))
		throw runtime_error("expected name");
	sb += parseName(it);
	sb += parseOptionString(parser, expand);
}

//returns true if the end of it is reached, else false
bool parseOptionValue(Parser& parser, StringBuilder& sb, bool expand)
{
	auto& it = parser.getIt();
	if (!it)
		return true;
	switch (*it)
	{
	case CHAR_COLON:
		sb += parseColon(parser, expand);
		return false;
	case CHAR_EQUAL:
		sb += parseEqual(parser, expand);
		return false;
	case CHAR_SCOPE:
		parseOptionScope(parser, sb, expand);
		return false;
	default:
		break;
	}
	while (isNameStart(*it))
	{
		sb += " -" + *it;
		if (!++it)
			return true;
	}
	if (isNameBody(*it))
		throw runtime_error(ERROR_OPTION);
	return false;
}

string webss::parseOptionLine(Parser& parser, std::function<bool(char c)> endCondition)
{
	auto& it = parser.getIt();
	const auto& aliases = parser.aliases;
	StringBuilder sb;
	while (it && !endCondition(*it))
	{
		if (*it == '-')
		{
			sb += *it;
			if (!++it)
				throw runtime_error(ERROR_OPTION);
			else if (isNameStart(*it))
			{
				sb += *it;
				++it;
				if (parseOptionValue(parser, sb, true))
					return sb;
				continue;
			}
			else if (*it == '-' && ++it && isNameStart(*it))
			{
				sb += '-' + parseName(it);
				if (parseOptionValue(parser, sb, true))
					return sb;
				continue;
			}
			throw runtime_error(ERROR_OPTION);
		}
		else if (isNameStart(*it))
		{
			sb += aliases.at(parseName(it));
			if (parseOptionValue(parser, sb, true))
				return sb;
			continue;
		}
		else if (isLineJunk(*it))
		{
			sb += *it;
			if (!++it)
				return sb;
			if (checkJunkOperators(it))
				continue;
		}

		sb += *it;
		++it;
	}
	return sb;
}

string expandOptionString(Parser& parser, string s)
{
	SmartIterator it(move(s));
	const auto& aliases = parser.aliases;
	StringBuilder sb;
	while (it)
	{
		if (*it == '-')
		{
			sb += *it;
			if (!++it)
				throw runtime_error(ERROR_OPTION);
			else if (isNameStart(*it))
			{
				sb += *it;
				++it;
				if (parseOptionValue(parser, sb, false))
					return sb;
				continue;
			}
			else if (*it == '-' && ++it && isNameStart(*it))
			{
				sb += '-' + parseName(it);
				if (parseOptionValue(parser, sb, false))
					return sb;
				continue;
			}
			throw runtime_error(ERROR_OPTION);
		}
		else if (isNameStart(*it))
		{
			sb += aliases.at(parseName(it));
			if (parseOptionValue(parser, sb, false))
				return sb;
			continue;
		}

		sb += *it;
		++it;
	}
	return sb;
}