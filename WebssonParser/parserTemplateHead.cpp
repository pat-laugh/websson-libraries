//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"

using namespace std;
using namespace webss;

const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

const ConType CON = ConType::TEMPLATE_HEAD;

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
		skipJunkToTag(++it, Tag::END_TEMPLATE);
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
		switch (other.abstractEntity.getContent().getTypeSafe())
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
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? Webss(move(thead), true) : Webss(parseTemplateHeadStandard(it, move(thead)), true);
		default:
			throw runtime_error("unexpected entity type within thead: " + other.abstractEntity.getContent().getTypeSafe().toString());
		}
	default:
		throw runtime_error(ERROR_ANONYMOUS_KEY);
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText(It& it)
{
	auto headWebss = parseTemplateHead(it);
	if (headWebss.getType() == WebssType::TEMPLATE_HEAD_STANDARD || headWebss.getType() == WebssType::TEMPLATE_HEAD_TEXT)
		return move(headWebss.getTemplateHeadStandard());
	else
		throw runtime_error("expected standard template head");
}

TemplateHeadBinary Parser::parseTemplateHeadBinary(It& it, TemplateHeadBinary&& thead)
{
	assert(it);
	do
		if (*it == OPEN_TUPLE)
			parseBinaryHead(++it, thead);
		else
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
	while (checkNextElementContainer(it, CON));
	return move(thead);
}

TemplateHeadScoped Parser::parseTemplateHeadScoped(It& it, TemplateHeadScoped&& thead)
{
	assert(it);
	do
		if (*it == CHAR_ABSTRACT_ENTITY)
			checkMultiContainer(++it, [&]() { thead.attach(ParamScoped::makeEntityAbstract(parseAbstractEntity(it, Namespace::getEmptyInstance()))); });
		else if (*it == CHAR_CONCRETE_ENTITY)
			checkMultiContainer(++it, [&]() { thead.attach(ParamScoped::makeEntityConcrete(parseConcreteEntity(it, CON))); });
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
		{
			if (++it != CHAR_COLON)
				throw runtime_error(webss_ERROR_EXPECTED_CHAR(CHAR_COLON));
			skipJunkToTag(++it, Tag::START_TEMPLATE);

			auto head = parseTemplateHeadText(++it);
			parseOtherValuesTheadStandardAfterThead(it, thead);
			thead.back().setTemplateHead(move(head), true);
		}
		else
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
	while (checkNextElementContainer(it, CON));
	return move(thead);
}

void Parser::parseStandardParameterTemplateHead(It& it, TemplateHeadStandard& thead)
{
	auto headWebss = parseTemplateHead(++it);
	parseOtherValuesTheadStandardAfterThead(it, thead);
	auto& lastParam = thead.back();
	switch (headWebss.getType())
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadBinary()));
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadScoped()));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandard()));
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandard()), true);
		break;
	default:
		throw logic_error("");
	}
}

void Parser::parseOtherValuesTheadStandardAfterThead(It& it, TemplateHeadStandard& thead)
{
	parseOtherValue(it, CON,
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
}

