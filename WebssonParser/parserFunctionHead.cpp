//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

const char ERROR_TEXT_FUNCTION_HEAD[] = "values in text function head must be of type string";
const char ERROR_BINARY_FUNCTION[] = "all values in a binary function must be binary";

#define CON ConType::FUNCTION_HEAD
#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)

FunctionHeadSwitch Parser::parseFunctionHead(It& it)
{
	if (checkEmptyContainer(it, CON))
		throw runtime_error(ERROR_EMPTY_FUNCTION_HEAD);

	switch (*it)
	{
	case OPEN_FUNCTION: case CHAR_COLON:
		return parseFunctionHeadStandard(it, FunctionHeadStandard());
	case OPEN_TUPLE:
		return parseFunctionHeadBinary(it, FunctionHeadBinary());
	case CHAR_VARIABLE: case CHAR_BLOCK: case CHAR_USING_NAMESPACE: //scoped function
	case OPEN_DICTIONARY: //mandatory function
	default:
		if (!isNameStart(*it))
			THROW_ERROR;
		break;
	}

	//if it's a variable, then the fhead is of the same type as the variable
	//if not, then the fhead is a standard fhead

	FunctionHeadStandard fhead;
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
		fhead.attach(move(keyPair.first), parseValue(it, CON));
		return parseFunctionHeadStandard(it, move(fhead));
	case KeyType::KEYWORD:
		throw runtime_error(ERROR_KEYWORD_KEY);
	case KeyType::KEYNAME:
		fhead.attachEmpty(move(keyPair.first));
		return parseFunctionHeadStandard(it, move(fhead));
	case KeyType::VARIABLE:
		return checkFunctionHeadType(it, vars[keyPair.first]);
	case KeyType::SCOPE:
		return checkFunctionHeadType(it, parseScopedValue(it, keyPair.first));
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}

FunctionHeadSwitch Parser::checkFunctionHeadType(It& it, const Variable& var)
{
	switch (var.getContent().getType())
	{
	case WebssType::FUNCTION_HEAD_STANDARD:
		return parseFunctionHeadStandard(it, FunctionHeadStandard(checkVariableFunctionHeadStandard(var.getName())));
	case WebssType::FUNCTION_HEAD_BINARY:
		return parseFunctionHeadBinary(it, FunctionHeadBinary(checkVariableFunctionHeadBinary(var.getName())));
	case WebssType::FUNCTION_HEAD_SCOPED:
	case WebssType::FUNCTION_HEAD_MANDATORY:
	default:
		THROW_ERROR;
	}
}

FunctionHeadSwitch Parser::checkFunctionHeadType(It& it, const Webss& webss)
{
	switch (webss.getType())
	{
	case WebssType::FUNCTION_HEAD_STANDARD:
		return parseFunctionHeadStandard(it, FunctionHeadStandard(webss.getFunctionHeadStandard()));
	case WebssType::FUNCTION_HEAD_BINARY:
		return parseFunctionHeadBinary(it, FunctionHeadBinary(webss.getFunctionHeadBinary()));
	case WebssType::FUNCTION_HEAD_SCOPED:
	case WebssType::FUNCTION_HEAD_MANDATORY:
	default:
		THROW_ERROR;
	}
}

FunctionHeadStandard Parser::parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead)
{
	do
	{
		switch (*it)
		{
		case OPEN_FUNCTION:
			parseStandardParameterFunctionHead(it, fhead);
			break;
		case CHAR_COLON:
			parseStandardParameterFunctionHeadText(it, fhead);
			break;
		case CLOSE_FUNCTION:
			checkContainerEnd(it);
			return move(fhead);
		default:
			if (checkOtherValues(it, [&]() { parseFunctionHeadNameStart(it, fhead); }, []() { THROW_ERROR; }))
				continue;
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_EXPECTED);
}

FunctionHeadBinary Parser::parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead)
{
	do
	{
		switch (*it)
		{
		case OPEN_TUPLE:
			parseBinaryHead(++it, fhead);
			break;
		case CLOSE_FUNCTION:
			checkContainerEnd(it);
			return move(fhead);
		default:
			if (checkOtherValues(it, [&]() { parseFunctionHeadBinaryNameStart(it, fhead); }, []() { THROW_ERROR; }))
				continue;
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_EXPECTED);
}

void Parser::parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead)
{
	using Type = FunctionHeadSwitch::Type;
	auto headSwitch = parseFunctionHead(++it);
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	parseFunctionHeadNameStart(it, fhead);
	auto& lastParam = fhead.back();
	switch (headSwitch.t)
	{
	case Type::STANDARD:
		lastParam.setFunctionHead(move(headSwitch.fheadStandard));
		return;
	case Type::BINARY:
		lastParam.setFunctionHead(move(headSwitch.fheadBinary));
		return;
	default:
		throw logic_error("");
	}
}

void Parser::parseStandardParameterFunctionHeadText(It& it, FunctionHeadStandard& fhead)
{
	if (++it != CHAR_COLON)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CHAR_COLON));
	skipJunkToValidCondition(++it, [&]() { return *it == OPEN_FUNCTION; });

	auto head = parseFunctionHeadText(it);
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	parseFunctionHeadTextNameStart(it, fhead);
	auto& lastParam = fhead.back();
	lastParam.setFunctionHead(move(head));
}

void Parser::parseFunctionHeadNameStart(It& it, FunctionHeadStandard& fhead)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
		fhead.attach(move(keyPair.first), parseValue(it, CON));
		return;
	case KeyType::KEYWORD:
		throw runtime_error(ERROR_KEYWORD_KEY);
	case KeyType::KEYNAME:
		fhead.attach(move(keyPair.first), ParamStandard());
		return;
	case KeyType::VARIABLE:
		fhead.attach(checkVariableFunctionHeadStandard(keyPair.first));
		return;
	case KeyType::SCOPE:
		fhead.attach(checkIsFunctionHeadStandard(parseScopedValue(it, keyPair.first)));
		return;
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}

void Parser::parseFunctionHeadTextNameStart(It& it, FunctionHeadStandard& fhead)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case KeyType::COLON:
		if (++it == CHAR_COLON)
			throw runtime_error(ERROR_TEXT_FUNCTION_HEAD);
		fhead.attach(move(keyPair.first), Webss(parseLineString(it, CON)));
		return;
	case KeyType::CSTRING:
		fhead.attach(move(keyPair.first), Webss(parseCString(++it)));
		return;
	case KeyType::KEYWORD:
		throw runtime_error(ERROR_KEYWORD_KEY);
	case KeyType::KEYNAME:
		fhead.attach(move(keyPair.first), Webss());
		return;
	default:
		throw runtime_error(ERROR_TEXT_FUNCTION_HEAD);
	}
}

void Parser::parseFunctionHeadBinaryNameStart(It& it, FunctionHeadBinary& fhead)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case KeyType::VARIABLE:
		fhead.attach(checkIsFunctionHeadBinary(vars[keyPair.first].getContent()));
		return;
	case KeyType::SCOPE:
		fhead.attach(checkIsFunctionHeadBinary(parseScopedValue(it, keyPair.first)));
		return;
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}

//////////////////////////////////////////////////

FunctionHeadStandard Parser::parseFunctionHeadText(It& it)
{
	if (checkEmptyContainer(it, CON))
		throw runtime_error(ERROR_EMPTY_FUNCTION_HEAD);

	FunctionHeadStandard fhead(true);
	do
	{
		switch (*it)
		{
		case CLOSE_FUNCTION:
			checkContainerEnd(it);
			return fhead;
		default:
			if (checkOtherValues(it, [&]() { parseFunctionHeadTextNameStart(it, fhead); }, []() { THROW_ERROR; }))
				continue;
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_EXPECTED);
}

#undef THROW_ERROR
#undef CON

