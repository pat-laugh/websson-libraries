//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

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
	case CHAR_CONCRETE_ENTITY: case CHAR_ABSTRACT_ENTITY: case CHAR_USING_NAMESPACE: //scoped function
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
		fhead.attach(move(keyPair.first), parseCharValue(it, CON));
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
		throw runtime_error(ERROR_UNEXPECTED);
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

void Parser::checkFheadVoid(It& it)
{
	//the first param of fheads is checked before calling the appropriate function, so the function
	//receives as if the first param was empty
	if (!it)
		throw runtime_error(ERROR_EXPECTED);
	isVoid = false;
}

#define ExtraCasesFheadStandard \
	ExtraCase(OPEN_FUNCTION, parseStandardParameterFunctionHead(it, fhead)) \
	ExtraCase(CHAR_COLON, parseStandardParameterFunctionHeadText(it, fhead))

FunctionHeadStandard Parser::parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead)
{
	PatternParse(checkFheadVoid(it), move(fhead), CON, CLOSE_FUNCTION, ExtraCasesFheadStandard, CheckOtherValues(parseFunctionHeadNameStart(it, fhead), THROW_ERROR))
}

FunctionHeadStandard Parser::parseFunctionHeadText(It& it)
{
	PatternParse(FunctionHeadStandard cont(true); CheckEmpty(CON, throw runtime_error(ERROR_EMPTY_FUNCTION_HEAD)), cont, CON, CLOSE_FUNCTION, , CheckOtherValues(parseFunctionHeadTextNameStart(it, cont), THROW_ERROR))
}

//similar to both standard fheads, but allows empty head
BlockHead Parser::parseBlockHead(It& it)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_FUNCTION: //regular
		PatternParse(++it; BlockHead fhead; CheckEmpty(CON, return fhead), fhead, CON, CLOSE_FUNCTION, ExtraCasesFheadStandard, CheckOtherValues(parseFunctionHeadNameStart(it, fhead), THROW_ERROR))
	case CHAR_COLON: //text
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		PatternParse(++it; BlockHead fhead(true); CheckEmpty(CON, return fhead), fhead, CON, CLOSE_FUNCTION, , CheckOtherValues(parseFunctionHeadTextNameStart(it, fhead), THROW_ERROR))
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

FunctionHeadBinary Parser::parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead)
{
	PatternParse(checkFheadVoid(it), move(fhead), CON, CLOSE_FUNCTION, ExtraCase(OPEN_TUPLE, parseBinaryHead(++it, fhead)), CheckOtherValues(parseFunctionHeadBinaryNameStart(it, fhead), THROW_ERROR))
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

	auto head = parseFunctionHeadText(++it);
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
		fhead.attach(move(keyPair.first), parseCharValue(it, CON));
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
		throw runtime_error(ERROR_UNEXPECTED);
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
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

#undef THROW_ERROR
#undef CON

