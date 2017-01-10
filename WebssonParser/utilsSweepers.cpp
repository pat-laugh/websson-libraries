//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsSweepers.h"

using namespace std;
using namespace webss;

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	for (; it; ++it)
		if (!isJunk(*it))
		{
			if (*it == CHAR_ESCAPE)
			{
				if (it.peekEnd())
					++it;
				else if (isJunk(it.peek()))
				{
					checkLineEscape(++it);
					skipJunk(it);
				}
			}
			else if (*it == CHAR_COMMENT && !it.peekEnd())
			{
				char c = it.peek();
				if (c == CHAR_COMMENT)
				{
					skipLine(it.readTwo());
					continue;
				}
				else if (c == '*')
					skipJunk(skipMultilineComment(it.readTwo()));
			}
			break;
		}
	return it;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(ERROR_EXPECTED);
	return it;
}

SmartIterator& webss::skipWhitespace(SmartIterator& it)
{
	for (; it; ++it)
		if (!isWhitespace(*it))
		{
			if (*it == CHAR_ESCAPE)
			{
				if (it.peekEnd())
					++it;
				else if (isJunk(it.peek()))
					checkLineEscape(++it);
			}
			else if (*it == CHAR_COMMENT && !it.peekEnd())
			{
				char c = it.peek();
				if (c == CHAR_COMMENT)
					skipLine(it.readTwo());
				else if (c == '*')
					skipMultilineComment(it.readTwo());
			}
			break;
		}
	return it;
}

SmartIterator& webss::skipLine(SmartIterator& it)
{
	for (; it; ++it)
		if (*it == '\n')
			break;
	return it;
}

SmartIterator& webss::skipMultilineComment(SmartIterator& it)
{
	int num = 1;
	for (; it; ++it)
		if (*it == '*')
		{
			if (!(++it))
				break;
			if (*it == '/' && --num == 0)
				return skipWhitespace(++it);
		}
		else if (*it == '/')
		{
			if (!(++it))
				break;
			if (*it == '*')
				++num;
		}
	throw runtime_error("comment is not closed");
}

void webss::cleanLine(SmartIterator& it, ConType con, Language lang)
{
	if (!skipWhitespace(it) || isLineEnd(*it, con, lang))
		return;
	throw runtime_error(ERROR_UNEXPECTED);
}

SmartIterator& webss::skipJunkToValidCondition(SmartIterator& it, function<bool()> condition)
{
	skipJunkToValid(it);
	if (!condition())
		throw runtime_error(ERROR_UNEXPECTED);
	return it;
}

void webss::checkLineEscape(SmartIterator& it)
{
	while (it && isWhitespace(*it))
		++it;
	if (!it)
		return;
	if (*it == '\n')
	{
		skipWhitespace(++it);
		return;
	}
	throw runtime_error(ERROR_UNEXPECTED);
}