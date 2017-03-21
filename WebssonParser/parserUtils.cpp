//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "utilsParser.h"

using namespace std;
using namespace webss;

const char* ERROR_VOID = "can't have void element";

bool GlobalParser::Parser::checkEmptyContainer(ConType con)
{
	if (checkContainerEnd(it, con))
		return true;
	if (*it == separator)
		throw runtime_error(ERROR_VOID);
	return false;
}

bool GlobalParser::Parser::checkNextElementContainer(ConType con)
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

bool GlobalParser::Parser::checkEmptyContainerVoid(ConType con, function<void()> funcIsVoid)
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

bool GlobalParser::Parser::checkNextElementContainerVoid(ConType con, function<void()> funcIsVoid)
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