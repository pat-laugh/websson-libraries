//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsSweepers.hpp"

#include <cassert>

#include "errors.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace various;
using namespace webss;

static void skipLineComment(SmartIterator& it);
static void skipMultilineComment(SmartIterator& it);
static void skipLineEscape(SmartIterator& it);

SmartIterator& webss::skipJunk(SmartIterator& it)
{
	while (it)
		if (isJunk(*it))
			++it;
		else if (!checkJunkOperators(it))
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
			checkJunkOperators(it);
			break;
		}
	return it;
}

SmartIterator& webss::skipLine(SmartIterator& it)
{
	for (; it; ++it)
		if (*it == '\n')
		{
			++it;
			break;
		}
	return it;
}

static void skipLineComment(SmartIterator& it)
{
	while (it && *it != '\n')
		++it;
}

static void skipMultilineComment(SmartIterator& it)
{
	if (!it)
		throw runtime_error(WEBSSON_EXCEPTION("multiline comment is not closed"));
	
	int num = 1;
	for (char c = *it; ++it; c = *it)
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
	
	throw runtime_error(WEBSSON_EXCEPTION("multiline comment is not closed"));
}

static void skipLineEscape(SmartIterator& it)
{
loopStart:
	if (!it)
		return;
	if (*it == '\n')
	{
		skipLineJunk(++it);
		return;
	}
	if (isLineJunk(*it))
	{
		++it;
		goto loopStart;
	}
	if (checkJunkOperators(it))
		goto loopStart;
	throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
}

bool webss::checkJunkOperators(SmartIterator& it)
{
	if (*it != CHAR_COMMENT || it.peekEnd())
		return false;

	switch (it.peek())
	{
	case CHAR_COMMENT:
		skipLineComment(it.incTwo());
		break;
	case '*':
		skipMultilineComment(it.incTwo());
		break;
	case '~':
		skipLineEscape(it.incTwo());
		break;
	default:
		return false;
	}
	return true;
}

SmartIterator& webss::skipJunkToValid(SmartIterator& it)
{
	if (!skipJunk(it))
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_EXPECTED));
	return it;
}

bool webss::isLineEnd(char c, ConType con)
{
	return c == '\n' || c == CHAR_SEPARATOR || con.isEnd(c);
}

static bool checkEndOfLine(SmartIterator& it)
{
	if (!it)
		return true;
	return *it == '\n';
}

bool webss::checkLineEmpty(SmartIterator& it)
{
	if (checkEndOfLine(it))
		return true;
	if (isJunk(*it))
		return checkEndOfLine(skipLineJunk(++it));
	return checkJunkOperators(it) && checkEndOfLine(it);
}

string webss::parseName(SmartIterator& it)
{
	assert(it && isNameStart(*it));
	string name;
	name += *it;
	while (++it)
		if (isNameBody(*it))
			name += *it;
		else if (isNameSeparator(*it) && it.peekGood() && isNameBody(it.peek()))
		{
			name += *it;
			name += *++it;
		}
		else
			break;
	return name;
}

void webss::checkNameExplicit(TagIterator& tagit)
{
	auto& it = ++tagit.getIt();
	if (!it || !isNameStart(*it))
		throw runtime_error(WEBSSON_EXCEPTION("expected name"));
	tagit.setTag(Tag::NAME_START);
}

string webss::parseNameExplicit(TagIterator& tagit)
{
	checkNameExplicit(tagit);
	return parseName(tagit.getItSafe());
}