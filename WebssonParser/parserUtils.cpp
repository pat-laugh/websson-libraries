//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"

using namespace std;
using namespace webss;

const char* ERROR_VOID = "can't have void element";

bool checkContainerEnd(SmartIterator& it, ConType con)
{
	if (!skipJunk(it))
	{
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return true;
	}
	if (con.isEnd(*it))
	{
		++it;
		return true;
	}
	return false;
}

bool GlobalParser::Parser::checkEmptyContainer(ConType con)
{
	if (checkContainerEnd(it, con))
		return true;
	if (*it == CHAR_SEPARATOR)
		throw runtime_error(ERROR_VOID);
	return false;
}

bool GlobalParser::Parser::checkNextElementContainer(ConType con)
{
	if (!lineGreed)
		cleanLine(it, con);
	else
		lineGreed = false;

	if (checkContainerEnd(it, con))
		return false;
	if (*it == CHAR_SEPARATOR && (checkContainerEnd(++it, con) || *it == CHAR_SEPARATOR))
		throw runtime_error(ERROR_VOID);
	return true;
}

bool GlobalParser::Parser::checkEmptyContainerVoid(ConType con, function<void()> funcIsVoid)
{
	if (checkContainerEnd(it, con))
		return true;
	if (*it == CHAR_SEPARATOR)
	{
		funcIsVoid();
	loopStart:
		if (checkContainerEnd(++it, con))
		{
			funcIsVoid();
			return true;
		}
		if (*it == CHAR_SEPARATOR)
		{
			funcIsVoid();
			goto loopStart;
		}
	}
	return false;
}

bool GlobalParser::Parser::checkNextElementContainerVoid(ConType con, function<void()> funcIsVoid)
{
	if (!lineGreed)
		cleanLine(it, con);
	else
		lineGreed = false;

	if (checkContainerEnd(it, con))
		return false;
	if (*it == CHAR_SEPARATOR)
	{
	loopStart:
		if (checkContainerEnd(++it, con))
		{
			funcIsVoid();
			return false;
		}
		if (*it == CHAR_SEPARATOR)
		{
			funcIsVoid();
			goto loopStart;
		}
	}
	return true;
}