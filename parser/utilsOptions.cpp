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

void parseColon(Parser& parser, StringBuilder& sb)
{
	++parser.getIt();
	sb += CHAR_COLON + parseLineString(parser);
}
void parseEqual(Parser& parser, StringBuilder& sb)
{
	++parser.getIt();
	sb += CHAR_EQUAL + parseStickyLineString(parser);
}

void parseOptionScope(Parser& parser, StringBuilder& sb)
{
	auto& it = parser.getIt();
	sb += CHAR_SCOPE;
	if (!++it || !isNameStart(*it))
		throw runtime_error("expected name");
	sb += parseName(it);
	if (!it)
		throw runtime_error("expected value");
	if (*it == CHAR_COLON)
		parseColon(parser, sb);
	else if (*it == CHAR_EQUAL)
		parseEqual(parser, sb);
	else
		throw runtime_error("expected value");
}

//returns true if the end of it is reached, else false
bool parseOptionValue(Parser& parser, StringBuilder& sb)
{
	auto& it = parser.getIt();
	if (!it)
		return true;
	switch (*it)
	{
	case CHAR_COLON:
		parseColon(parser, sb);
		return false;
	case CHAR_EQUAL:
		parseEqual(parser, sb);
		return false;
	case CHAR_SCOPE:
		parseOptionScope(parser, sb);
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
				if (parseOptionValue(parser, sb))
					return sb;
				continue;
			}
			else if (*it == '-' && ++it && isNameStart(*it))
			{
				sb += '-' + parseName(it);
				if (parseOptionValue(parser, sb))
					return sb;
				continue;
			}
			throw runtime_error(ERROR_OPTION);
		}
		else if (isNameStart(*it))
		{
			sb += aliases.at(parseName(it));
			if (parseOptionValue(parser, sb))
				return sb;
			continue;
		}
		else if (*it == CHAR_COMMENT)
		{
			if (checkJunkOperators(it))
				continue;
		}

		sb += *it;
		++it;
	}
	return sb;
}