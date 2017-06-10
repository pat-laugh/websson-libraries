//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsOptions.hpp"

#include "errors.hpp"
#include "iteratorSwitcher.hpp"
#include "parserStrings.hpp"
#include "utilsSweepers.hpp"
#include "utils/constants.hpp"
#include "utils/stringBuilder.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

const char ERROR_OPTION[] = "expected option";

string parseOptionString(Parser& parser)
{
	auto& it = parser.getIt();
	if (!it)
		throw runtime_error("expected value");
	char c = *it;
	++it;
	if (c == CHAR_COLON)
		return parseLineString(parser);
	else if (c == CHAR_EQUAL)
		return parseStickyLineString(parser);
	else
		throw runtime_error("expected value");
}

void parseOptionScope(Parser& parser, vector<string>& output)
{
	auto& it = parser.getIt();
	output.push_back(OPTION_SCOPE);
	if (!++it || !isNameStart(*it))
		throw runtime_error("expected name");
	output.push_back(parseName(it));
	output.push_back(OPTION_VALUE);
	output.push_back(parseOptionString(parser));
}

//returns true if the end of it is reached, else false
bool parseOptionValue(Parser& parser, vector<string>& output)
{
	auto& it = parser.getIt();
	if (!it)
		return true;
	if (*it == CHAR_COLON || *it == CHAR_EQUAL)
	{
		output.push_back(OPTION_VALUE);
		output.push_back(parseOptionString(parser));
		return false;
	}
	else if (*it == CHAR_SCOPE)
	{
		parseOptionScope(parser, output);
		return false;
	}
	while (isNameStart(*it))
	{
		output.push_back(OPTION_NAME);
		output.push_back(string() + *it);
		if (!++it)
			return true;
	}
	if (isNameBody(*it))
		throw runtime_error(ERROR_OPTION);
	return false;
}

vector<string> webss::parseOptionLine(Parser& parser, std::function<bool(char c)> endCondition)
{
	auto& it = parser.getIt();
	StringBuilder sb;
	while (it && !endCondition(*it))
	{
		if (isLineJunk(*it))
		{
			sb += *it;
			if (!++it)
				break;
			checkJunkOperators(it);
			continue;
		}
		sb += *it;
		++it;
	}
	return expandOptionString(parser, sb);
}

vector<string> webss::expandOptionString(Parser& parser, string s)
{
	auto& it = parser.getIt();
	IteratorSwitcher itSwitcher(it, SmartIterator(move(s)));
	const auto& aliases = parser.aliases;
	vector<string> output;
	while (it)
	{
		if (*it == '-')
		{
			if (!++it)
				throw runtime_error(ERROR_OPTION);
			else if (isNameStart(*it))
			{
				output.push_back(OPTION_NAME);
				output.push_back(string() + *it);
				++it;
				if (parseOptionValue(parser, output))
					break;
				continue;
			}
			else if (*it == '-' && ++it && isNameStart(*it))
			{
				output.push_back(OPTION_NAME);
				output.push_back(parseName(it));
				if (parseOptionValue(parser, output))
					break;
				continue;
			}
			throw runtime_error(ERROR_OPTION);
		}
		else if (isNameStart(*it))
		{
			for (const auto& item : aliases.at(parseName(it)))
				output.push_back(item);
			if (parseOptionValue(parser, output))
				break;
			continue;
		}
		else if (isJunk(*it))
		{
			if (!++it)
				break;
			checkJunkOperators(it);
			continue;
		}
		else
			throw runtime_error(ERROR_UNEXPECTED);
	}
	return output;
}