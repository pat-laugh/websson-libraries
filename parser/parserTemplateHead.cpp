//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utils/constants.hpp"

using namespace std;
using namespace webss;

const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

void parseStandardParameterTemplateHead(Parser& parser, TemplateHeadStandard& thead);

Webss Parser::parseTemplateHead(bool allowSelf)
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return TemplateHeadStandard();

	switch (*tagit)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return parseTemplateHeadStandard();
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary();
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a thead must be within a non-empty thead");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TemplateHeadSelf();
	case Tag::EXPAND:
		break;
	}

	//the thead is of the same type as the entity being expanded
	auto ent = parseExpandEntity(tagit, ents);
	bool isEnd = !checkNextElement();
	auto type = ent.getContent().getType();
	switch (type)
	{
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		return{ isEnd ? TemplateHeadBinary(ent) : parseTemplateHeadBinary(TemplateHeadBinary(ent)), type };
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		return{ isEnd ? TemplateHeadStandard(ent) : parseTemplateHeadStandard(TemplateHeadStandard(ent)), type };
	default:
		throw runtime_error("expand entity in template head must be a template head");
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText()
{
	auto webssThead = parseTemplateHead();
	if (!webssThead.isTemplateHeadStandard())
		throw runtime_error("expected standard template head");
	return move(webssThead.getTemplateHeadStandardRaw());
}

Webss Parser::parseTemplatePlusHead()
{
	auto webssThead = parseTemplateHead();
	switch (webssThead.getTypeRaw())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		return{ TemplateHeadBinary(move(webssThead.getTemplateHeadBinaryRaw())), WebssType::TEMPLATE_HEAD_PLUS_BINARY };
	case WebssType::TEMPLATE_HEAD_STANDARD: 
		return{ TemplateHeadStandard(move(webssThead.getTemplateHeadStandardRaw())), WebssType::TEMPLATE_HEAD_PLUS_STANDARD };
	case WebssType::TEMPLATE_HEAD_TEXT:
		return{ TemplateHeadStandard(move(webssThead.getTemplateHeadStandardRaw())), WebssType::TEMPLATE_HEAD_PLUS_TEXT };
	case WebssType::TEMPLATE_HEAD_PLUS_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		return webssThead;
	default:
		assert(false); throw domain_error("");
	}
}

TemplateHeadStandard Parser::parseTemplatePlusHeadText()
{
	return parseTemplateHeadText();
}

TemplateHeadBinary Parser::parseTemplateHeadBinary(TemplateHeadBinary&& thead)
{
	do
		if (*tagit == Tag::START_TUPLE)
			parseBinaryHead(thead);
		else if (*tagit == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (!ent.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(ent);
		}
		else
			throw runtime_error(ERROR_BINARY_TEMPLATE);
	while (checkNextElement());
	return move(thead);
}

void parseOtherValuesTheadStandardAfterThead(Parser& parser, TemplateHeadStandard& thead)
{
	parser.parseExplicitKeyValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); });
}

TemplateHeadStandard Parser::parseTemplateHeadStandard(TemplateHeadStandard&& thead)
{
	do
		if (*tagit == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(*this, thead);
		else if (*tagit == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(*this, thead);
			thead.back().setTemplateHead(move(head), WebssType::TEMPLATE_HEAD_TEXT);
		}
		else if (*tagit == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (!ent.getContent().isTemplateHeadStandard())
				throw runtime_error("expand entity within standard template head must be a standard template head");
			thead.attach(ent);
		}
		else
		{
			parseExplicitKeyValue(
				CaseKeyValue{ thead.attach(move(key), move(value)); },
				CaseKeyOnly{ thead.attachEmpty(move(key)); });
		}
	while (checkNextElement());
	return move(thead);
}

void parseStandardParameterTemplateHead(Parser& parser, TemplateHeadStandard& thead)
{
	auto webssThead = parser.parseTemplateHead(true);
	parseOtherValuesTheadStandardAfterThead(parser, thead);
	auto& lastParam = thead.back();
	auto type = webssThead.getTypeRaw();
	switch (webssThead.getTypeRaw())
	{
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		lastParam.setTemplateHead(move(webssThead.getTemplateHeadBinaryRaw()), type);
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		lastParam.setTemplateHead(move(webssThead.getTemplateHeadStandardRaw()), type);
		break;
	default:
		assert(false);
	}
}