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
void parseOtherValuesTheadStandardAfterThead(Parser& parser, TemplateHeadStandard& thead);

Webss Parser::parseTemplateHead(bool allowSelf)
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return TemplateHeadStandard();

	switch (nextTag)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return parseTemplateHeadStandard();
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary();
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a thead must be within a non-empty thead");
		skipJunkToTag(++it, Tag::END_TEMPLATE);
		++it;
		return TemplateHeadSelf();
	case Tag::EXPAND:
		break;
	}

	//the thead is of the same type as the entity being expanded
	auto ent = parseExpandEntity(it, ents);
	bool isEnd = !checkNextElement();
	auto type = ent.getContent().getType();
	switch (type)
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		return isEnd ? TemplateHeadBinary(ent) : parseTemplateHeadBinary(TemplateHeadBinary(ent));
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		return Webss(isEnd ? TemplateHeadStandard(ent) : parseTemplateHeadStandard(TemplateHeadStandard(ent)), type);
	default:
		throw runtime_error("expand entity in template head must be a template head");
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText()
{
	auto headWebss = parseTemplateHead();
	if (headWebss.getTypeRaw() != WebssType::TEMPLATE_HEAD_STANDARD && headWebss.getTypeRaw() != WebssType::TEMPLATE_HEAD_TEXT)
		throw runtime_error("expected standard template head");
	return move(headWebss.getTemplateHeadStandardRaw());
}

Webss Parser::parseTemplateValueHead()
{
	auto headWebss = parseTemplateHead();
	switch (headWebss.getTypeRaw())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		return{ TemplateHeadBinary(move(headWebss.getTemplateHeadBinaryRaw())), WebssType::TEMPLATE_VALUE_HEAD_BINARY };
	case WebssType::TEMPLATE_HEAD_STANDARD: 
		return{ TemplateHeadStandard(move(headWebss.getTemplateHeadStandardRaw())), WebssType::TEMPLATE_VALUE_HEAD_STANDARD };
	case WebssType::TEMPLATE_HEAD_TEXT:
		return{ TemplateHeadStandard(move(headWebss.getTemplateHeadStandardRaw())), WebssType::TEMPLATE_VALUE_HEAD_TEXT };
	default:
		assert(false); throw domain_error("");
	}
}

TemplateHeadStandard Parser::parseTemplateValueHeadText()
{
	return parseTemplateHeadText();
}

TemplateHeadBinary Parser::parseTemplateHeadBinary(TemplateHeadBinary&& thead)
{
	do
		if (nextTag == Tag::START_TUPLE)
			parseBinaryHead(thead);
		else if (nextTag == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(it, ents);
			if (!ent.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(ent);
		}
		else
			throw runtime_error(ERROR_BINARY_TEMPLATE);
	while (checkNextElement());
	return move(thead);
}

TemplateHeadStandard Parser::parseTemplateHeadStandard(TemplateHeadStandard&& thead)
{
	do
		if (nextTag == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(*this, thead);
		else if (nextTag == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(*this, thead);
			thead.back().setTemplateHead(move(head), WebssType::TEMPLATE_HEAD_TEXT);
		}
		else if (nextTag == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(it, ents);
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
	auto headWebss = parser.parseTemplateHead(true);
	parseOtherValuesTheadStandardAfterThead(parser, thead);
	auto& lastParam = thead.back();
	switch (headWebss.getTypeRaw())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadBinaryRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandardRaw()), headWebss.getTypeRaw());
		break;
	default:
		assert(false);
	}
}

void parseOtherValuesTheadStandardAfterThead(Parser& parser, TemplateHeadStandard& thead)
{
	parser.nextTag = getTag(++parser.getIt());
	parser.parseExplicitKeyValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); });
}