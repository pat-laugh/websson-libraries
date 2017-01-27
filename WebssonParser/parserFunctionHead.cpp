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

Webss Parser::parseFunctionHead(It& it)
{
	if (checkEmptyContainer(it, CON))
		return BlockHead();

	switch (*it)
	{
	case OPEN_FUNCTION: case CHAR_COLON:
		return parseFunctionHeadStandard(it);
	case OPEN_TUPLE:
		return parseFunctionHeadBinary(it);
	case CHAR_CONCRETE_ENTITY: case CHAR_ABSTRACT_ENTITY: case CHAR_USING_NAMESPACE:
		return parseFunctionHeadScoped(it);
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
		case WebssType::BLOCK_HEAD:
			if (!isEnd)
				throw runtime_error(ERROR_UNEXPECTED);
			return BlockHead(checkEntBlockHead(other.abstractEntity));
		case WebssType::FUNCTION_HEAD_BINARY:
		{
			FunctionHeadBinary fheadBinary(checkEntFheadBinary(other.abstractEntity));
			return isEnd ? move(fheadBinary) : parseFunctionHeadBinary(it, move(fheadBinary));
		}
		case WebssType::FUNCTION_HEAD_SCOPED:
		{
			FunctionHeadScoped fheadScoped(checkEntFheadScoped(other.abstractEntity));
			return isEnd ? move(fheadScoped) : parseFunctionHeadScoped(it, move(fheadScoped));
		}
		case WebssType::FUNCTION_HEAD_STANDARD:
			fhead = FunctionHeadStandard(checkEntFheadStandard(other.abstractEntity));
			return isEnd ? move(fhead) : parseFunctionHeadStandard(it, move(fhead));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	default:
		THROW_ERROR;
	}
}

FunctionHeadBinary Parser::parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead)
{
	do
		if (*it == OPEN_TUPLE)
			parseBinaryHead(++it, fhead);
		else
			parseOtherValuesFheadBinary(it, fhead);
	while (checkNextElementContainer(it, CON));
	return move(fhead);
}

FunctionHeadScoped Parser::parseFunctionHeadScoped(It& it, FunctionHeadScoped&& fhead)
{
/*	auto& docHead = static_cast<vector<ParamDocument>>(*const_cast<FunctionHeadScoped::Parameters*>(&fhead.getParameters()));
	do
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseAbstractEntity(it, Namespace::getEmptyInstance())); });
			break;
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseConcreteEntity(it, CON)); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseUsingNamespaceStatic(it)); });
			break;
		case CHAR_IMPORT:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseImportStatic(it, CON)); });
			break;
		default:
			if (!isNameStart(*it))
				throw runtime_error(ERROR_UNEXPECTED);
			auto nameType = parseNameType(it);
			if (nameType.type != NameType::ENTITY)
				throw runtime_error("expected function head scoped entity");
			fhead.attach(checkEntFheadScoped(nameType.entity));
			break;
		}
	} while (checkNextElementContainer(it, CON));
	ParamDocumentIncluder().removeAll(static_cast<vector<ParamDocument>>(fhead.getParameters()));
	return move(fhead);*/
	return move(fhead);
}

FunctionHeadStandard Parser::parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead)
{
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

FunctionHeadText Parser::parseFunctionHeadText(It& it)
{
	FunctionHeadText fhead;
	if (checkEmptyContainer(it, CON))
		throw runtime_error("text function head can't be empty");
	do
		parseOtherValuesFheadText(it, fhead);
	while (checkNextElementContainer(it, CON));
	return fhead;
}

void Parser::parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead)
{
	auto headWebss = parseFunctionHead(++it);
	parseOtherValuesFheadStandardParam(it, fhead);
	auto& lastParam = fhead.back();
	switch (headWebss.t)
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::FUNCTION_HEAD_BINARY:
		lastParam.setFunctionHead(move(*headWebss.fheadBinary));
		break;
	case WebssType::FUNCTION_HEAD_SCOPED:
		lastParam.setFunctionHead(move(*headWebss.fheadScoped));
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		lastParam.setFunctionHead(move(*headWebss.fheadStandard));
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

void Parser::parseOtherValuesFheadText(It& it, FunctionHeadText& fhead)
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

