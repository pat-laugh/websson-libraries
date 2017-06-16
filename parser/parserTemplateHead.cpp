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

TheadOptions Parser::parseTheadOptions()
{
	TheadOptions options;
	auto& it = getIt();
	while (*skipJunkToValid(++it) != CLOSE_LIST)
	{
		if (*it == CHAR_THEAD_PLUS)
			options.isPlus = true;
		else if (*it == CHAR_COLON && ++it && *it == CHAR_COLON)
			options.isText = true;
		else
			throw runtime_error(ERROR_UNEXPECTED);
	}
	++tagit;
	return options;
}

Thead Parser::parseThead(bool allowSelf)
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return TemplateHeadStandard();


	switch (*tagit)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return parseTheadStandard();
	case Tag::START_TUPLE:
		return parseTheadBinary();
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a thead must be within a non-empty thead");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TemplateHeadSelf();
	case Tag::START_LIST:
	{
		TheadOptions options = parseTheadOptions();
		switch (*tagit)
		{
		case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
		default:
			return{ parseTheadStandard(), options };
		case Tag::START_TUPLE:
			return{ parseTheadBinary(), options };
		case Tag::SELF:
			if (!allowSelf)
				throw runtime_error("self in a thead must be within a non-empty thead");
			(++tagit).sofertTag(Tag::END_TEMPLATE);
			++tagit;
			return{ TemplateHeadSelf(), options };
		case Tag::START_LIST:
			throw runtime_error("template head options must be at the start only");
		case Tag::EXPAND:
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (ent.getContent().getType() != WebssType::TEMPLATE_HEAD)
				throw runtime_error("expand entity in template head must be a template head");
				return ent;
			const auto& thead = ent.getContent().getThead();
			auto typeThead = thead.getType();
			assert(typeThead == TheadType::BINARY || typeThead == TheadType::STANDARD);
			if (typeThead == TheadType::BINARY)
				return{ !checkNextElement() ? TemplateHeadBinary(thead.getTheadBinary()) : parseTheadBinary(TemplateHeadBinary(thead.getTheadBinary())), options };
			else
				return{ !checkNextElement() ? TemplateHeadStandard(thead.getTheadStandard()) : parseTheadStandard(TemplateHeadStandard(thead.getTheadStandard())), options };
		}
		}
	}
	case Tag::EXPAND:
		break;
	}

	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::TEMPLATE_HEAD)
		throw runtime_error("expand entity in template head must be a template head");
	if (!checkNextElement())
		return ent;
	const auto& thead = ent.getContent().getThead();
	auto options = thead.getOptions();
	auto typeThead = thead.getType();
	assert(typeThead == TheadType::BINARY || typeThead == TheadType::STANDARD);
	if (typeThead == TheadType::BINARY)
		return{ parseTheadBinary(TemplateHeadBinary(thead.getTheadBinary())), options };
	else
		return{ parseTheadStandard(TemplateHeadStandard(thead.getTheadStandard())), options };
}

/*
Thead Parser::parseTheadText()
{
	auto thead = parseThead();
	if (!thead.isTheadStandard())
		throw runtime_error("expected standard template head");
	return move(webssThead.getTheadStandardRaw());
}

Webss Parser::parseTemplateHeadPlus()
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

TemplateHeadStandard Parser::parseTemplateHeadPlusText()
{
	return parseTemplateHeadText();
}*/

TemplateHeadBinary Parser::parseTheadBinary(TemplateHeadBinary&& thead)
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

TemplateHeadStandard Parser::parseTheadStandard(TemplateHeadStandard&& thead)
{
	do
		if (*tagit == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(*this, thead);
	/*	else if (*tagit == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(*this, thead);
			thead.back().setTemplateHead(move(head), WebssType::TEMPLATE_HEAD_TEXT);
		}*/
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
	auto webssThead = parser.parseThead(true);
	parseOtherValuesTheadStandardAfterThead(parser, thead);
	auto& lastParam = thead.back();
	auto type = webssThead.getTypeRaw();
	switch (webssThead.getTypeRaw())
	{
	case TheadType::SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case TheadType::BINARY:
		lastParam.setTemplateHead(move(webssThead.getTheadBinaryRaw()));
		break;
	case TheadType::STANDARD:
		lastParam.setTemplateHead(move(webssThead.getTheadStandardRaw()));
		break;
	default:
		assert(false);
	}
}