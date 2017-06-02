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

Webss Parser::parseTemplateHead()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return BlockHead();

	switch (nextTag)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return parseTemplateHeadStandard();
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary();
	case Tag::SELF:
		skipJunkToTag(++it, Tag::END_TEMPLATE);
		++it;
		return TemplateHeadSelf();
	case Tag::EXPAND:
		break;
	}

	//the thead is of the same type as the entity being expanded
	auto ent = parseExpandEntity(it, ents);
	bool isEnd = !checkNextElement();
	switch (ent.getContent().getType())
	{
	case WebssType::BLOCK_HEAD:
		if (!isEnd)
			throw runtime_error(ERROR_UNEXPECTED);
		return BlockHead(ent);
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		TemplateHeadBinary theadBinary(ent);
		return isEnd ? move(theadBinary) : parseTemplateHeadBinary(move(theadBinary));
	}
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		TemplateHeadStandard thead(ent);
		return isEnd ? move(thead) : parseTemplateHeadStandard(move(thead));
	}
	case WebssType::TEMPLATE_HEAD_TEXT:
	{
		TemplateHeadStandard thead(ent);
		return isEnd ? Webss(move(thead), true) : Webss(parseTemplateHeadStandard(move(thead)), true);
	}
	default:
		throw runtime_error("expand entity in template head must be a template head");
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText()
{
	auto headWebss = parseTemplateHead();
	if (headWebss.getTypeRaw() == WebssType::TEMPLATE_HEAD_STANDARD || headWebss.getTypeRaw() == WebssType::TEMPLATE_HEAD_TEXT)
		return move(headWebss.getTemplateHeadStandardRaw());
	throw runtime_error("expected standard template head");
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
			thead.back().setTemplateHead(move(head), true);
		}
		else if (nextTag == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(it, ents);
			if (!ent.getContent().isTemplateHeadStandard())
				throw runtime_error("can't expand " + ent.getContent().getType().toString() + " within standard template head");
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
	auto headWebss = parser.parseTemplateHead();
	parseOtherValuesTheadStandardAfterThead(parser, thead);
	auto& lastParam = thead.back();
	switch (headWebss.getTypeRaw())
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadBinaryRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandardRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandardRaw()), true);
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