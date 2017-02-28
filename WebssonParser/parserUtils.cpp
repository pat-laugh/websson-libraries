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

bool Parser::checkEmptyContainer(It& it, ConType con)
{
	if (checkContainerEnd(it, con))
		return true;
	if (*it == separator)
		throw runtime_error(ERROR_VOID);
	return false;
}

bool Parser::checkNextElementContainer(It& it, ConType con)
{
	if (!lineGreed)
		cleanLine(it, con, separator);
	else
		lineGreed = false;

	if (checkContainerEnd(it, con))
		return false;
	if (*it == separator && (checkContainerEnd(++it, con) || *it == separator))
		throw runtime_error(ERROR_VOID);
	return true;
}

bool Parser::checkEmptyContainerVoid(It& it, ConType con, function<void()> funcIsVoid)
{
	if (checkContainerEnd(it, con))
		return true;
	if (*it == separator)
	{
		funcIsVoid();
	loopStart:
		if (checkContainerEnd(++it, con))
		{
			funcIsVoid();
			return true;
		}
		if (*it == separator)
		{
			funcIsVoid();
			goto loopStart;
		}
	}
	return false;
}

bool Parser::checkNextElementContainerVoid(It& it, ConType con, function<void()> funcIsVoid)
{
	if (!lineGreed)
		cleanLine(it, con, separator);
	else
		lineGreed = false;

	if (checkContainerEnd(it, con))
		return false;
	if (*it == separator)
	{
	loopStart:
		if (checkContainerEnd(++it, con))
		{
			funcIsVoid();
			return false;
		}
		if (*it == separator)
		{
			funcIsVoid();
			goto loopStart;
		}
	}
	return true;
}