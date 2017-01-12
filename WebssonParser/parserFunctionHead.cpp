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
	case OPEN_DICTIONARY: //mandatory function
	default:
		break;
	}

	//if it's a variable, then the fhead is of the same type as the variable
	//if not, then the fhead is a standard fhead

	FunctionHeadStandard fhead;
	auto other = parseOtherValue(it, CON);
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		fhead.attach(move(other.key), move(other.value));
		return parseFunctionHeadStandard(it, move(fhead));
	case OtherValue::Type::KEY_ONLY:
		fhead.attachEmpty(move(other.key));
		return parseFunctionHeadStandard(it, move(fhead));
	case OtherValue::Type::ABSTRACT_ENTITY:
		switch (other.abstractEntity.getContent().getType())
		{
		case WebssType::FUNCTION_HEAD_BINARY:
			return parseFunctionHeadBinary(it, FunctionHeadBinary(checkVarFheadBinary(other.abstractEntity)));
		case WebssType::FUNCTION_HEAD_MANDATORY:
			break;
		case WebssType::FUNCTION_HEAD_SCOPED:
			break;
		case WebssType::FUNCTION_HEAD_STANDARD:
			return parseFunctionHeadStandard(it, FunctionHeadStandard(checkVarFheadStandard(other.abstractEntity)));
		default:
			break;
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
	isVoid = false;
}

#define ExtraCasesFheadStandard \
	ExtraCase(OPEN_FUNCTION, parseStandardParameterFunctionHead(it, fhead)) \
	ExtraCase(CHAR_COLON, parseStandardParameterFunctionHeadText(it, fhead))

FunctionHeadStandard Parser::parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead)
{
	PatternParse(checkFheadVoid(it), move(fhead), CON, CLOSE_FUNCTION, ExtraCasesFheadStandard, parseOtherValuesFheadStandard(it, fhead);)
}

FunctionHeadStandard Parser::parseFunctionHeadText(It& it)
{
	PatternParse(FunctionHeadStandard cont(true); CheckEmpty(CON, throw runtime_error(ERROR_EMPTY_FUNCTION_HEAD)), cont, CON, CLOSE_FUNCTION, , parseOtherValuesFheadText(it, cont);)
}

//similar to both standard fheads, but allows empty head
BlockHead Parser::parseBlockHead(It& it)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_FUNCTION: //regular
		PatternParse(++it; BlockHead fhead; CheckEmpty(CON, return fhead), fhead, CON, CLOSE_FUNCTION, ExtraCasesFheadStandard, parseOtherValuesFheadStandard(it, fhead);)
	case CHAR_COLON: //text
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		PatternParse(++it; BlockHead fhead(true); CheckEmpty(CON, return fhead), fhead, CON, CLOSE_FUNCTION, , parseOtherValuesFheadText(it, fhead);)
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

FunctionHeadBinary Parser::parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead)
{
	PatternParse(checkFheadVoid(it), move(fhead), CON, CLOSE_FUNCTION, ExtraCase(OPEN_TUPLE, parseBinaryHead(++it, fhead)), parseOtherValuesFheadBinary(it, fhead);)
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
	case Type::MANDATORY:
		//...
	case Type::SCOPE:
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
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAlias{ fhead.attachAlias(move(key)); });
}

void Parser::parseOtherValuesFheadStandard(It& it, FunctionHeadStandard& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ fhead.attach(move(key), move(value)); },
		CaseKeyOnly{ fhead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ fhead.attach(checkVarFheadStandard(abstractEntity)); },
		CaseAlias{ fhead.attachAlias(move(key)); });
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
		CaseAbstractEntity{ throw runtime_error(ERROR_TEXT_FUNCTION_HEAD); },
		CaseAlias{ fhead.attachAlias(move(key)); });
}

void Parser::parseOtherValuesFheadBinary(It& it, FunctionHeadBinary& fhead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ throw runtime_error(ERROR_BINARY_FUNCTION); },
		CaseKeyOnly{ throw runtime_error(ERROR_BINARY_FUNCTION); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ fhead.attach(checkVarFheadBinary(abstractEntity)); },
		CaseAlias{ throw runtime_error(ERROR_BINARY_FUNCTION); });
}

#undef THROW_ERROR
#undef CON

