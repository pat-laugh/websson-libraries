//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"

using namespace std;
using namespace webss;

const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

Webss GlobalParser::Parser::parseTemplateHead()
{
	Parser parser(*this, ConType::TEMPLATE_HEAD, false);
	if (parser.parserContainerEmpty())
		return BlockHead();

	switch (nextTag)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
		return parser.parseTemplateHeadStandard();
	case Tag::START_TUPLE:
		return parser.parseTemplateHeadBinary();
	case Tag::ENTITY_ABSTRACT: case Tag::ENTITY_CONCRETE: case Tag::USING_NAMESPACE:
		return parser.parseTemplateHeadScoped();
	case Tag::SELF:
		skipJunkToTag(++it, Tag::END_TEMPLATE);
		++it;
		return TemplateHeadSelf();
	default:
		break;
	}

	//if it's a entity, then the thead is of the same type as the entity
	//if not, then the thead is a standard thead

	TemplateHeadStandard thead;
	auto other = parser.parseOtherValue();
	bool isEnd = !parser.parserCheckNextElement();
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		thead.attach(move(other.key), move(other.value));
		return isEnd ? move(thead) : parser.parseTemplateHeadStandard(move(thead));
	case OtherValue::Type::KEY_ONLY:
		thead.attachEmpty(move(other.key));
		return isEnd ? move(thead) : parser.parseTemplateHeadStandard(move(thead));
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
			return isEnd ? move(theadBinary) : parser.parseTemplateHeadBinary(move(theadBinary));
		}
		case WebssType::TEMPLATE_HEAD_SCOPED:
		{
			TemplateHeadScoped theadScoped(other.abstractEntity);
			return isEnd ? move(theadScoped) : parser.parseTemplateHeadScoped(move(theadScoped));
		}
		case WebssType::TEMPLATE_HEAD_STANDARD:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? move(thead) : parser.parseTemplateHeadStandard(move(thead));
		case WebssType::TEMPLATE_HEAD_TEXT:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? Webss(move(thead), true) : Webss(parser.parseTemplateHeadStandard(move(thead)), true);
		default:
			throw runtime_error("unexpected entity type within thead: " + other.abstractEntity.getContent().getTypeSafe().toString());
		}
	default:
		throw runtime_error(ERROR_ANONYMOUS_KEY);
	}
}

TemplateHeadStandard GlobalParser::Parser::parseTemplateHeadText()
{
	auto headWebss = parseTemplateHead();
	if (headWebss.getType() == WebssType::TEMPLATE_HEAD_STANDARD || headWebss.getType() == WebssType::TEMPLATE_HEAD_TEXT)
		return move(headWebss.getTemplateHeadStandard());
	else
		throw runtime_error("expected standard template head");
}

TemplateHeadBinary GlobalParser::Parser::parseTemplateHeadBinary(TemplateHeadBinary&& thead)
{
	assert(it);
	do
		if (nextTag == Tag::START_TUPLE)
			parseBinaryHead(thead);
		else
			parseOtherValue(
				CaseKeyValue{ throw runtime_error(ERROR_BINARY_TEMPLATE); },
				CaseKeyOnly{ throw runtime_error(ERROR_BINARY_TEMPLATE); },
				CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
				CaseAbstractEntity
				{
					if (!abstractEntity.getContent().isTemplateHeadBinary())
						throw runtime_error(ERROR_BINARY_TEMPLATE);
					thead.attach(abstractEntity);
				});
	while (parserCheckNextElement());
	return move(thead);
}

TemplateHeadScoped GlobalParser::Parser::parseTemplateHeadScoped(TemplateHeadScoped&& thead)
{
	assert(it);
	do
		if (nextTag == Tag::ENTITY_ABSTRACT)
		{
			++it;
			thead.attach(ParamScoped::makeEntityAbstract(parseAbstractEntity(Namespace::getEmptyInstance())));
		}
		else if (nextTag == Tag::ENTITY_CONCRETE)
		{
			++it;
			thead.attach(ParamScoped::makeEntityConcrete(parseConcreteEntity()));
		}
		else if (nextTag == Tag::USING_NAMESPACE)
		{
			++it;
			thead.attach(ParamScoped(parseUsingNamespaceStatic()));
		}
		else
			parseOtherValue(
				CaseKeyValue{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseKeyOnly{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseAbstractEntity
				{
					if (!abstractEntity.getContent().isTemplateHeadScoped())
						throw runtime_error(ERROR_BINARY_TEMPLATE);
					thead.attach(abstractEntity);
				});
	while (parserCheckNextElement());
	return move(thead);
}

TemplateHeadStandard GlobalParser::Parser::parseTemplateHeadStandard(TemplateHeadStandard&& thead)
{
	assert(it);
	do
		if (nextTag == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(thead);
		else if (nextTag == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(thead);
			thead.back().setTemplateHead(move(head), true);
		}
		else
		{
			parseOtherValue(
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
	while (parserCheckNextElement());
	return move(thead);
}

void GlobalParser::Parser::parseStandardParameterTemplateHead(TemplateHeadStandard& thead)
{
	auto headWebss = parseTemplateHead();
	parseOtherValuesTheadStandardAfterThead(thead);
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

void GlobalParser::Parser::parseOtherValuesTheadStandardAfterThead(TemplateHeadStandard& thead)
{
	lineGreed = true;
	parserCheckNextElement();
	parseOtherValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
}

