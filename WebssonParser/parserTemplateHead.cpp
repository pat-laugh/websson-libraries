//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

const char ERROR_TEXT_TEMPLATE_HEAD[] = "values in text template head must be of type string";
const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

const ConType CON = ConType::TEMPLATE_HEAD;

#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)
#define THROW_ERROR_ANONYMOUS_KEY throw runtime_error(ERROR_ANONYMOUS_KEY)
#define THROW_ERROR_TEXT_TEMPLATE_HEAD throw runtime_error(ERROR_TEXT_TEMPLATE_HEAD)
#define THROW_ERROR_BINARY_TEMPLATE throw runtime_error(ERROR_BINARY_TEMPLATE)

Webss Parser::parseTemplateHead(It& it)
{
	if (checkEmptyContainer(it, CON))
		return BlockHead();

	switch (*it)
	{
	case OPEN_TEMPLATE: case CHAR_COLON:
		return parseTemplateHeadStandard(it);
	case OPEN_TUPLE:
		return parseTemplateHeadBinary(it);
	case CHAR_CONCRETE_ENTITY: case CHAR_ABSTRACT_ENTITY: case CHAR_USING_NAMESPACE:
		return parseTemplateHeadScoped(it);
	case OPEN_DICTIONARY:
		throw runtime_error("this parser cannot parse mandatory templates");
	case CHAR_SELF:
		skipJunkToValidCondition(++it, [&]() { return *it == CLOSE_TEMPLATE; });
		++it;
		return TemplateHeadSelf();
	default:
		break;
	}

	//if it's a entity, then the thead is of the same type as the entity
	//if not, then the thead is a standard thead

	TemplateHeadStandard thead;
	auto other = parseOtherValue(it, CON);
	bool isEnd = !checkNextElementContainer(it, CON);
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		thead.attach(move(other.key), move(other.value));
		return isEnd ? move(thead) : parseTemplateHeadStandard(it, move(thead));
	case OtherValue::Type::KEY_ONLY:
		thead.attachEmpty(move(other.key));
		return isEnd ? move(thead) : parseTemplateHeadStandard(it, move(thead));
	case OtherValue::Type::ABSTRACT_ENTITY:
		switch (other.abstractEntity.getContent().getType())
		{
		case WebssType::BLOCK_HEAD:
			if (!isEnd)
				throw runtime_error(ERROR_UNEXPECTED);
			return BlockHead(other.abstractEntity);
		case WebssType::TEMPLATE_HEAD_BINARY:
		{
			TemplateHeadBinary theadBinary(other.abstractEntity);
			return isEnd ? move(theadBinary) : parseTemplateHeadBinary(it, move(theadBinary));
		}
		case WebssType::TEMPLATE_HEAD_SCOPED:
		{
			TemplateHeadScoped theadScoped(other.abstractEntity);
			return isEnd ? move(theadScoped) : parseTemplateHeadScoped(it, move(theadScoped));
		}
		case WebssType::TEMPLATE_HEAD_STANDARD:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? move(thead) : parseTemplateHeadStandard(it, move(thead));
		case WebssType::TEMPLATE_HEAD_TEXT:
		{
			TemplateHeadText theadText(other.abstractEntity);
			return isEnd ? move(theadText) : parseTemplateHeadText(it, move(theadText));
		}
		default:
			throw runtime_error("unexpected entity type within thead: " + other.abstractEntity.getContent().getType().toString());
		}
	default:
		THROW_ERROR;
	}
}

TemplateHeadBinary Parser::parseTemplateHeadBinary(It& it, TemplateHeadBinary&& thead)
{
	assert(it);
	do
		if (*it == OPEN_TUPLE)
			parseBinaryHead(++it, thead);
		else
			parseOtherValuesFheadBinary(it, thead);
	while (checkNextElementContainer(it, CON));
	return move(thead);
}

TemplateHeadScoped Parser::parseTemplateHeadScoped(It& it, TemplateHeadScoped&& thead)
{
	assert(it);
	do
		if (*it == CHAR_ABSTRACT_ENTITY)
			checkMultiContainer(++it, [&]() { thead.attach(ParamScoped(parseAbstractEntity(it, Namespace::getEmptyInstance()))); });
		else if (*it == CHAR_CONCRETE_ENTITY)
			checkMultiContainer(++it, [&]() { thead.attach(ParamScoped(parseConcreteEntity(it, CON), true)); });
		else if (*it == CHAR_USING_NAMESPACE)
			checkMultiContainer(++it, [&]() { thead.attach(ParamScoped(parseUsingNamespaceStatic(it))); });
		else
			parseOtherValue(it, CON,
				CaseKeyValue{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseKeyOnly{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseAbstractEntity
				{
					if (!abstractEntity.getContent().isTemplateHeadScoped())
						throw runtime_error(ERROR_BINARY_TEMPLATE);
					thead.attach(abstractEntity);
				});
	while (checkNextElementContainer(it, CON));
	return move(thead);
}

TemplateHeadStandard Parser::parseTemplateHeadStandard(It& it, TemplateHeadStandard&& thead)
{
	assert(it);
	do
		if (*it == OPEN_TEMPLATE)
			parseStandardParameterTemplateHead(it, thead);
		else if (*it == CHAR_COLON)
			parseStandardParameterTemplateHeadText(it, thead);
		else
			parseOtherValuesFheadStandard(it, thead);
	while (checkNextElementContainer(it, CON));
	return move(thead);
}

TemplateHeadText Parser::parseTemplateHeadText(It& it, TemplateHeadText&& thead)
{
	if (checkEmptyContainer(it, CON))
		throw runtime_error("text template head can't be empty");
	do
		parseOtherValuesFheadText(it, thead);
	while (checkNextElementContainer(it, CON));
	return thead;
}

void Parser::parseStandardParameterTemplateHead(It& it, TemplateHeadStandard& thead)
{
	auto headWebss = parseTemplateHead(++it);
	parseOtherValuesFheadStandardAfterFhead(it, thead);
	auto& lastParam = thead.back();
	switch (headWebss.type)
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(*headWebss.theadBinary));
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		lastParam.setTemplateHead(move(*headWebss.theadScoped));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		lastParam.setTemplateHead(move(*headWebss.theadStandard));
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(*headWebss.theadText));
		break;
	default:
		throw logic_error("");
	}
}

void Parser::parseStandardParameterTemplateHeadText(It& it, TemplateHeadStandard& thead)
{
	if (++it != CHAR_COLON)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CHAR_COLON));
	skipJunkToValidCondition(++it, [&]() { return *it == OPEN_TEMPLATE; });

	auto head = parseTemplateHeadText(++it);
	parseOtherValuesFheadStandardAfterFhead(it, thead);
	thead.back().setTemplateHead(move(head));
}

void Parser::parseOtherValuesFheadStandardAfterFhead(It& it, TemplateHeadStandard& thead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
}

void Parser::parseOtherValuesFheadStandard(It& it, TemplateHeadStandard& thead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity
		{
			if (!abstractEntity.getContent().isTemplateHeadStandard())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(abstractEntity);
		});
}

void Parser::parseOtherValuesFheadText(It& it, TemplateHeadText& thead)
{
	parseOtherValue(it, CON,
		CaseKeyValue
		{
			if (!value.isString())
				throw runtime_error(ERROR_TEXT_TEMPLATE_HEAD);
			thead.attach(move(key), move(value));
		},
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity
		{
			if (!abstractEntity.getContent().isTemplateHeadText())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(abstractEntity);
		});
}

void Parser::parseOtherValuesFheadBinary(It& it, TemplateHeadBinary& thead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ throw runtime_error(ERROR_BINARY_TEMPLATE); },
		CaseKeyOnly{ throw runtime_error(ERROR_BINARY_TEMPLATE); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity
		{
			if (!abstractEntity.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(abstractEntity);
		});
}

#undef THROW_ERROR
#undef CON

