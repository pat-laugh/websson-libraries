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

TemplateHeadStandard parseTemplateHeadStandard(Parser& parser, TemplateHeadStandard&& thead = TemplateHeadStandard());
TemplateHeadBinary parseTemplateHeadBinary(Parser& parser, TemplateHeadBinary&& thead = TemplateHeadBinary());
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
		return parseTemplateHeadStandard(*this);
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary(*this);
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
		return isEnd ? move(theadBinary) : parseTemplateHeadBinary(*this, move(theadBinary));
	}
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		TemplateHeadStandard thead(ent);
		return isEnd ? move(thead) : parseTemplateHeadStandard(*this, move(thead));
	}
	case WebssType::TEMPLATE_HEAD_TEXT:
	{
		TemplateHeadStandard thead(ent);
		return isEnd ? Webss(move(thead), true) : Webss(parseTemplateHeadStandard(*this, move(thead)), true);
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

TemplateHeadBinary parseTemplateHeadBinary(Parser& parser, TemplateHeadBinary&& thead)
{
	do
		if (parser.nextTag == Tag::START_TUPLE)
			parser.parseBinaryHead(thead);
		else if (parser.nextTag == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(parser.getIt(), parser.getEnts());
			if (!ent.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(ent);
		}
		else
			throw runtime_error(ERROR_BINARY_TEMPLATE);
	while (parser.checkNextElement());
	return move(thead);
}

Dictionary Parser::parseDictionary(bool isAbstract)
{
	return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), isAbstract, [&](Dictionary& dict)
	{
		if (nextTag == Tag::EXPAND)
			expandDictionary(dict, it, ents, isAbstract);
		else
		{
			string name = parseNameDictionary();
			nextTag = getTag(it);
			dict.addSafe(move(name), parseValueOnly());
		}
	});
}

TemplateHeadStandard parseTemplateHeadStandard(Parser& parser, TemplateHeadStandard&& thead)
{
	do
		if (parser.nextTag == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(parser, thead);
		else if (parser.nextTag == Tag::TEXT_TEMPLATE)
		{
			auto head = parser.parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(parser, thead);
			thead.back().setTemplateHead(move(head), true);
		}
		else if (parser.nextTag == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(parser.getIt(), parser.getEnts());
			if (!ent.getContent().isTemplateHeadStandard())
				throw runtime_error("can't expand " + ent.getContent().getType().toString() + " within standard template head");
			thead.attach(ent);
		}
		else
		{
			parser.parseOtherValue(
				CaseKeyValue{ thead.attach(move(key), move(value)); },
				CaseKeyOnly{ thead.attachEmpty(move(key)); },
				ErrorValueOnly(ERROR_ANONYMOUS_KEY),
				ErrorAbstractEntity(ERROR_UNEXPECTED));
		}
	while (parser.checkNextElement());
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
	parser.checkNextElement();
	parser.parseOtherValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		ErrorValueOnly(ERROR_ANONYMOUS_KEY),
		ErrorAbstractEntity(ERROR_UNEXPECTED));
}