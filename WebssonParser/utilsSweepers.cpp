//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsSweepers.h"

using namespace std;
using namespace webss;

bool checkOperators(SmartIterator& it);
void skipLineComment(SmartIterator& it);
void skipMultilineComment(SmartIterator& it);

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	while (it)
		if (isJunk(*it))
			++it;
		else if (!checkOperators(it))
			break;
	return it;
}

SmartIterator& webss::skipLineJunk(SmartIterator& it)
{
	while (it)
		if (isLineJunk(*it))
			++it;
		else
		{
			checkOperators(it);
			break;
		}
	return it;
}

void skipLineComment(SmartIterator& it)
{
	while (it && *it != '\n')
		++it;
}

void skipMultilineComment(SmartIterator& it)
{
	if (!it)
		throw runtime_error("multiline comment is not closed");
	
	for (int num = 1, char c = *it; ++it; c = *it)
		if (c == '*')
		{
			if (*it != '/')
				continue;
			if (--num == 0)
			{
				skipLineJunk(++it);
				return;
			}
			if (!++it)
				break;
		}
		else if (c == '/' && *it == '*')
		{
			++num;
			if (!++it)
				break;
		}
	
	throw runtime_error("multiline comment is not closed");
}

bool checkOperators(SmartIterator& it)
{
	if (*it == CHAR_ESCAPE)
		return checkLineEscape(it);
	else if (*it == CHAR_COMMENT)
		return checkComment(it);
	return false;
}

bool webss::checkLineEscape(SmartIterator& it)
{
	if (it.peekEnd())
	{
		++it;
		return true;
	}
	if (!isJunk(it.peek()))
		return false;
	
	while (++it && isLineJunk(*it))
		;
	if (!it)
		return true;
	if (*it != '\n')
		throw runtime_error(ERROR_UNEXPECTED);
	skipLineJunk(++it);
	return true;
}

bool webss::checkComment(SmartIterator& it)
{
	if (it.peekEnd())
		return false;
	
	char c = it.peek();
	if (c == CHAR_COMMENT)
		skipLineComment(it.readTwo());
	else if (c == '*')
		skipMultilineComment(it.readTwo());
	else
		return false;
	return true;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(ERROR_EXPECTED);
	return it;
}

SmartIterator& webss::skipJunkToValidCondition(SmartIterator& it, function<bool()> condition)
{
	skipJunkToValid(it);
	if (!condition())
		throw runtime_error(ERROR_UNEXPECTED);
	return it;
}

void webss::cleanLine(SmartIterator& it, ConType con, Language lang)
{
	if (skipLineJunk(it) && !isLineEnd(*it, con, lang))
		throw runtime_error(ERROR_UNEXPECTED);
}