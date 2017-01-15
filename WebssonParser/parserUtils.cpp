//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

const char* ERROR_VOID = "can't have void element";

bool checkContainerHasEndChar(It& it, ConType con)
{
	if (con.hasEndChar())
		throw runtime_error(ERROR_EXPECTED);
	return true;
}

bool checkContainerEnd(It& it, ConType con)
{
	if (!con.isEnd(*it))
		return false;

	++it;
	return true;
}

bool Parser::checkEmptyContainer(It& it, ConType con)
{
	if (!skipJunk(it))
		return checkContainerHasEndChar(it, con);
	if (*it == separator)
		throw runtime_error(ERROR_VOID);
	return checkContainerEnd(it, con);
}

bool Parser::checkNextElementContainer(It& it, ConType con)
{
	if (!lineGreed)
		cleanLine(it, con, separator);
	else
		lineGreed = false;

	if (!skipJunk(it))
		return checkContainerHasEndChar(it, con);
	if (*it == separator)
	{
		if (!skipJunk(++it) || *it == separator || con.isEnd(*it))
			throw runtime_error(ERROR_VOID);
		return true;
	}
	return !checkContainerEnd(it, con);
}


bool Parser::checkSeparator(It& it)
{
	return checkSeparatorVoid(it, []() { throw runtime_error(ERROR_VOID); });
}

bool Parser::checkSeparatorVoid(It& it, function<void()> funcIsVoid)
{
	if (*it == separator)
	{
		if (isVoid)
			funcIsVoid();
		isVoid = true;
		skipJunk(++it);
		return true;
	}
	return false;
}

void Parser::checkContainerEndVoid(It& it, function<void()> funcIsVoid)
{
	if (isVoid)
		funcIsVoid();
	lineGreed = false;
	++it;
}

bool Parser::checkEmptyContainerVoid(It& it, ConType con)
{
	if (con.isEnd(*skipJunkToValid(it)))
	{
		++it;
		return true;
	}
	isVoid = true;
	return false;
}

void Parser::checkToNextElement(It& it, ConType con)
{
	if (!lineGreed)
		cleanLine(it, con, separator);
	else
		lineGreed = false;
	isVoid = false;
	skipJunk(it);
}

bool Parser::checkOtherValuesVoid(It& it, function<void()> funcIsVoid, function<void()> funcIsNameStart, function<void()> funcIsNumberStart)
{
	if (checkSeparatorVoid(it, move(funcIsVoid)))
		return true;
	else if (isNameStart(*it))
		funcIsNameStart();
	else if (isNumberStart(*it))
		funcIsNumberStart();
	else
		throw runtime_error(ERROR_UNEXPECTED);
	return false;
}

const BasicEntity<FunctionHeadStandard>& Parser::checkEntFheadStandard(const Entity& ent)
{
	const auto& name = ent.getName();
	if (!entsFheadStandard.hasEntity(name))
		try { entsFheadStandard.add(name, ent.getContent().getFunctionHeadStandard()); }
		catch (exception e) { throw runtime_error(e.what()); }
	return entsFheadStandard[name];
}

const BasicEntity<FunctionHeadBinary>& Parser::checkEntFheadBinary(const Entity& ent)
{
	const auto& name = ent.getName();
	if (!entsFheadBinary.hasEntity(name))
		try { entsFheadBinary.add(name, ent.getContent().getFunctionHeadBinary()); }
		catch (exception e) { throw runtime_error(e.what()); }
	return entsFheadBinary[name];
}