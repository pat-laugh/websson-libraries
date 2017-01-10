//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "unicode.h"

using namespace std;
using namespace webss;

const char* ERROR_VOID = "empty value";

//adds the char corresponding to an escape; for serialization
//REQUIREMENT: the char must be an escapable char
void addEscapedChar(string& str, char c)
{
	switch (c)
	{
	case '0': str += '\0'; break;
	case 'a': str += '\a'; break;
	case 'b': str += '\b'; break;
	case 'f': str += '\f'; break;
	case 'n': str += '\n'; break;
	case 'r': str += '\r'; break;
	case 't': str += '\t'; break;
	case 'v': str += '\v'; break;

	case 's': str += ' '; //no need for break
	case 'e': break; //empty

	default: //isSpecialAscii, else undefined behavior
		str += c;
	}
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
	checkToNextElementVoid(it, con);
}

void Parser::checkToNextElementVoid(It& it, ConType con)
{
	if (!lineGreed)
		cleanLine(it, con, language);
	else
		lineGreed = false;
	isVoid = false;
	skipJunk(it);
}

bool Parser::checkOtherValues(It& it, function<void()> funcIsNameStart, function<void()> funcIsNumberStart)
{
	return checkOtherValuesVoid(it, []() { throw runtime_error(ERROR_VOID); }, move(funcIsNameStart), move(funcIsNumberStart));
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

void Parser::checkEscapedChar(It& it, std::string& line, function<void()> funcIsSENT)
{
	if (!(++it))
		return; //as if were followed by empty line
	if (!isEscapableChar(*it))
	{
		checkLineEscape(it);
		return;
	}

	switch (*it)
	{
	case 'x': case 'X': case 'u': case 'U':
		addEscapedHex(it, line, language);
		break;
	case 's': case 'e': case 'n': case 't':
		funcIsSENT();
	default:
		addEscapedChar(line, *it);
		++it;
	}
}

bool Parser::checkVariableString(It& it, string& line)
{
	if (!isNameStart(*it))
	{
		line += CHAR_VARIABLE;
		return false;
	}
	line += parseVariableString(it);
	return true;
}

const BasicVariable<FunctionHeadStandard>& Parser::checkVariableFunctionHeadStandard(const string& name)
{
	if (!varsFunctionHeadStandard.hasVariable(name))
		try { varsFunctionHeadStandard.add(name, vars[name].getContent().getFunctionHeadStandard()); }
		catch (exception e) { throw runtime_error(e.what()); }
	return varsFunctionHeadStandard[name];
}

const BasicVariable<FunctionHeadBinary>& Parser::checkVariableFunctionHeadBinary(const string& name)
{
	if (!varsFunctionHeadBinary.hasVariable(name))
		try { varsFunctionHeadBinary.add(name, vars[name].getContent().getFunctionHeadBinary()); }
	catch (exception e) { throw runtime_error(e.what()); }
	return varsFunctionHeadBinary[name];
}

const BasicVariable<type_int>& Parser::checkVariableTypeInt(const string& name)
{
	if (!varsTypeInt.hasVariable(name))
		try { varsTypeInt.add(name, vars[name].getContent().getInt()); }
	catch (exception e) { throw runtime_error(e.what()); }
	return varsTypeInt[name];
}