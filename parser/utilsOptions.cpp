//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsOptions.hpp"

#include "errors.hpp"
#include "iteratorSwitcher.hpp"
#include "parserStrings.hpp"
#include "utilsSweepers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "various/stringBuilder.hpp"

using namespace std;
using namespace various;
using namespace webss;

const std::string webss::OPTION_NAME = "-", webss::OPTION_SCOPE = ".", webss::OPTION_VALUE = ":";
static const char* ERROR_OPTION = "expected option";

static bool isOptionStringChar(char c)
{
	return c == CHAR_COLON || c == CHAR_EQUAL || c == CHAR_CSTRING;
}

static string parseOptionString(Parser& parser)
{
	auto& it = parser.getIt();
	assert(it && isOptionStringChar(*it));
	if (*it == CHAR_CSTRING)
		return parseCString(parser);
	char c = *it;
	++it;
	return c == CHAR_COLON ? parseLineString(parser) : parseStickyLineString(parser);
}

static void parseOptionScope(Parser& parser, vector<string>& output)
{
	auto& it = parser.getIt();
	output.push_back(OPTION_SCOPE);
	if (!++it || !isNameStart(*it))
		throw runtime_error("expected name");
	output.push_back(parseName(it));
	if (!it || !isOptionStringChar(*it))
		throw runtime_error("expected value");
	output.push_back(OPTION_VALUE);
	output.push_back(parseOptionString(parser));
}

//returns true if the end of it is reached, else false
static bool parseOptionValue(Parser& parser, vector<string>& output)
{
	auto& it = parser.getIt();
	if (!it)
		return true;
	if (isOptionStringChar(*it))
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
		if (isJunk(*it))
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