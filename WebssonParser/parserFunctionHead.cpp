//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

const char ERROR_TEXT_FUNCTION_HEAD[] = "values in text function head must be of type string";
const char ERROR_BINARY_FUNCTION[] = "all values in a binary function must be binary";

const ConType CON = ConType::FUNCTION_HEAD;

#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)
#define THROW_ERROR_ANONYMOUS_KEY throw runtime_error(ERROR_ANONYMOUS_KEY)
#define THROW_ERROR_TEXT_FUNCTION_HEAD throw runtime_error(ERROR_TEXT_FUNCTION_HEAD)
#define THROW_ERROR_BINARY_FUNCTION throw runtime_error(ERROR_BINARY_FUNCTION)

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
		//...
	case OPEN_DICTIONARY:
		throw runtime_error("this parser cannot parse mandatory functions");
	default:
		break;
	}

	//if it's a entity, then the fhead is of the same type as the entity
	//if not, then the fhead is a standard fhead

	FunctionHeadStandard fhead;
	auto other = parseOtherValue(it, CON);
	bool isEnd = !checkNextElementContainer(it, CON);
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		fhead.attach(move(other.key), move(other.value));
		return isEnd ? move(fhead) : parseFunctionHeadStandard(it, move(fhead));
	case OtherValue::Type::KEY_ONLY:
		fhead.attachEmpty(move(other.key));
		return isEnd ? move(fhead) : parseFunctionHeadStandard(it, move(fhead));
	case OtherValue::Type::ABSTRACT_ENTITY:
		switch (other.abstractEntity.getContent().getType())
		{
		case WebssType::FUNCTION_HEAD_BINARY:
		{
			FunctionHeadBinary fheadBinary(FunctionHeadBinary(checkEntFheadBinary(other.abstractEntity)));
			return isEnd ? move(fheadBinary) : parseFunctionHeadBinary(it, move(fheadBinary));
		}
		case WebssType::FUNCTION_HEAD_SCOPED:
			break;
		case WebssType::FUNCTION_HEAD_STANDARD:
			new (&fhead) FunctionHeadStandard(checkEntFheadStandard(other.abstractEntity));
			return isEnd ? move(fhead) : parseFunctionHeadStandard(it, move(fhead));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
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
	if (*it == separator)
		skipJunkToValid(it);
}

FunctionHeadStandard Parser::parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead)
{
//	checkFheadVoid(it);
	do
		if (*it == OPEN_FUNCTION)
			parseStandardParameterFunctionHead(it, fhead);
		else if (*it == CHAR_COLON)
			parseStandardParameterFunctionHeadText(it, fhead);
		else
			parseOtherValuesFheadStandard(it, fhead);
	while (checkNextElementContainer(it, CON));
	return move(fhead);
}

FunctionHeadStandard Parser::parseFunctionHeadText(It& it)
{
	FunctionHeadStandard fhead(true);
	if (checkEmptyContainer(it, CON))
		throw runtime_error(ERROR_EMPTY_FUNCTION_HEAD);
	do
		parseOtherValuesFheadText(it, fhead);
	while (checkNextElementContainer(it, CON));
	return fhead;
}

//similar to both standard fheads, but allows empty head
BlockHead Parser::parseBlockHead(It& it)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_FUNCTION: //regular
	{
		++it;
		BlockHead fhead;
		if (checkEmptyContainer(it, CON))
			return fhead;
		do
			if (*it == OPEN_FUNCTION)
				parseStandardParameterFunctionHead(it, fhead);
			else if (*it == CHAR_COLON)
				parseStandardParameterFunctionHeadText(it, fhead);
			else
				parseOtherValuesFheadStandard(it, fhead);
		while (checkNextElementContainer(it, CON));
		return fhead;
	}
	case CHAR_COLON: //text
	{
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");

		++it;
		BlockHead fhead;
		if (checkEmptyContainer(it, CON))
			return fhead;
		do
			parseOtherValuesFheadText(it, fhead);
		while (checkNextElementContainer(it, CON));
		return fhead;
	}
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

FunctionHeadBinary Parser::parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead)
{
//	checkFheadVoid(it);
	do
		if (*it == OPEN_TUPLE)
			parseBinaryHead(++it, fhead);
		else
			parseOtherValuesFheadBinary(it, fhead);
	while (checkNextElementContainer(it, CON));
	return move(fhead);
}

void Parser::parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead)
{
	using Type = FunctionHeadSwitch::Type;
	auto headSwitch = parseFunctionHead(++it);
	parseOtherValuesFheadStandardParam(it, fhead);
	auto& lastParam = fhead.back();
	switch (headSwitch.t)
	{
	case Type::BINARY:
		lastParam.setFunctionHead(move(headSwitch.fheadBinary));
		break;
	case Type::SCOPED:
		//...
	case Type::STANDARD:
		lastParam.setFunctionHead(move(headSwitch.fheadStandard));
		break;
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
	parseOtherValuesFheadStandardParam(it, fhead);
	fhead.back().setFunctionHead(move(head));
}

void Parser::parseOtherValuesFheadStandardParam(It& it, FunctionHeadStandard& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ fhead.attach(move(key), move(value)); },
		CaseKeyOnly{ fhead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
}

void Parser::parseOtherValuesFheadStandard(It& it, FunctionHeadStandard& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ fhead.attach(move(key), move(value)); },
		CaseKeyOnly{ fhead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ fhead.attach(checkEntFheadStandard(abstractEntity)); });
}

void Parser::parseOtherValuesFheadText(It& it, FunctionHeadStandard& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue
		{
			if (!value.isString())
				throw runtime_error(ERROR_TEXT_FUNCTION_HEAD);
			fhead.attach(move(key), move(value));
		},
		CaseKeyOnly{ fhead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_TEXT_FUNCTION_HEAD); });
}

void Parser::parseOtherValuesFheadBinary(It& it, FunctionHeadBinary& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ throw runtime_error(ERROR_BINARY_FUNCTION); },
		CaseKeyOnly{ throw runtime_error(ERROR_BINARY_FUNCTION); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ fhead.attach(checkEntFheadBinary(abstractEntity)); });
}

#undef THROW_ERROR
#undef CON

