//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

const char* ERROR_VOID = "empty value";

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

void Parser::checkContainerEnd(It& it)
{
	checkContainerEndVoid(it, []() { throw runtime_error(ERROR_VOID); });
}

void Parser::checkContainerEndVoid(It& it, function<void()> funcIsVoid)
{
	if (isVoid)
		funcIsVoid();
	lineGreed = false;
	++it;
}

bool Parser::checkEmptyContainer(It& it, ConType con)
{
	return checkEmptyContainerVoid(it, con);
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
		cleanLine(it, con, language);
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

const BasicVariable<FunctionHeadStandard>& Parser::checkVarFheadStandard(const Variable& var)
{
	const auto& name = var.getName();
	if (!varsFheadStandard.hasVariable(name))
		try { varsFheadStandard.add(name, var.getContent().getFunctionHeadStandard()); }
		catch (exception e) { throw runtime_error(e.what()); }
	return varsFheadStandard[name];
}

const BasicVariable<FunctionHeadBinary>& Parser::checkVarFheadBinary(const Variable& var)
{
	const auto& name = var.getName();
	if (!varsFheadBinary.hasVariable(name))
		try { varsFheadBinary.add(name, var.getContent().getFunctionHeadBinary()); }
		catch (exception e) { throw runtime_error(e.what()); }
	return varsFheadBinary[name];
}