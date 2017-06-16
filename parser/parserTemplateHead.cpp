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

const char ERROR_BIN_TEMPLATE[] = "all values in a binary template must be binary";

void parseStdParamThead(Parser& parser, TheadStd& thead);

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
	ContainerSwitcher switcher(*this, ConType::THEAD, false);
	if (containerEmpty())
		return TheadStd();


	switch (*tagit)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return parseTheadStd();
	case Tag::START_TUPLE:
		return parseTheadBin();
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a thead must be within a non-empty thead");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TheadSelf();
	case Tag::START_LIST:
	{
		TheadOptions options = parseTheadOptions();
		switch (*tagit)
		{
		case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
		default:
			return{ parseTheadStd(), options };
		case Tag::START_TUPLE:
			return{ parseTheadBin(), options };
		case Tag::SELF:
			if (!allowSelf)
				throw runtime_error("self in a thead must be within a non-empty thead");
			(++tagit).sofertTag(Tag::END_TEMPLATE);
			++tagit;
			return{ TheadSelf(), options };
		case Tag::START_LIST:
			throw runtime_error("template head options must be at the start only");
		case Tag::EXPAND:
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (ent.getContent().getType() != WebssType::THEAD)
				throw runtime_error("expand entity in template head must be a template head");
				return ent;
			const auto& thead = ent.getContent().getThead();
			auto typeThead = thead.getType();
			assert(typeThead == TypeThead::BIN || typeThead == TypeThead::STD);
			if (typeThead == TypeThead::BIN)
				return{ !checkNextElement() ? TheadBin(thead.getTheadBin()) : parseTheadBin(TheadBin(thead.getTheadBin())), options };
			else
				return{ !checkNextElement() ? TheadStd(thead.getTheadStd()) : parseTheadStd(TheadStd(thead.getTheadStd())), options };
		}
		}
	}
	case Tag::EXPAND:
		break;
	}

	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::THEAD)
		throw runtime_error("expand entity in template head must be a template head");
	if (!checkNextElement())
		return ent;
	const auto& thead = ent.getContent().getThead();
	auto options = thead.getOptions();
	auto typeThead = thead.getType();
	assert(typeThead == TypeThead::BIN || typeThead == TypeThead::STD);
	if (typeThead == TypeThead::BIN)
		return{ parseTheadBin(TheadBin(thead.getTheadBin())), options };
	else
		return{ parseTheadStd(TheadStd(thead.getTheadStd())), options };
}

/*
Thead Parser::parseTheadText()
{
	auto thead = parseThead();
	if (!thead.isTheadStd())
		throw runtime_error("expected standard template head");
	return move(webssThead.getTheadStdRaw());
}

Webss Parser::parseTheadPlus()
{
	auto webssThead = parseThead();
	switch (webssThead.getTypeRaw())
	{
	case WebssType::THEAD_BIN:
		return{ TheadBin(move(webssThead.getTheadBinRaw())), WebssType::THEAD_PLUS_BIN };
	case WebssType::THEAD_STD: 
		return{ TheadStd(move(webssThead.getTheadStdRaw())), WebssType::THEAD_PLUS_STD };
	case WebssType::THEAD_TEXT:
		return{ TheadStd(move(webssThead.getTheadStdRaw())), WebssType::THEAD_PLUS_TEXT };
	case WebssType::THEAD_PLUS_BIN: case WebssType::THEAD_PLUS_STD: case WebssType::THEAD_PLUS_TEXT:
		return webssThead;
	default:
		assert(false); throw domain_error("");
	}
}

TheadStd Parser::parseTheadPlusText()
{
	return parseTheadText();
}*/

TheadBin Parser::parseTheadBin(TheadBin&& thead)
{
	do
		if (*tagit == Tag::START_TUPLE)
			parseBinHead(thead);
		else if (*tagit == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (!ent.getContent().isTheadBin())
				throw runtime_error(ERROR_BIN_TEMPLATE);
			thead.attach(ent);
		}
		else
			throw runtime_error(ERROR_BIN_TEMPLATE);
	while (checkNextElement());
	return move(thead);
}

void parseOtherValuesTheadStdAfterThead(Parser& parser, TheadStd& thead)
{
	parser.parseExplicitKeyValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); });
}

TheadStd Parser::parseTheadStd(TheadStd&& thead)
{
	do
		if (*tagit == Tag::START_TEMPLATE)
			parseStdParamThead(*this, thead);
	/*	else if (*tagit == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTheadText();
			parseOtherValuesTheadStdAfterThead(*this, thead);
			thead.back().setThead(move(head), WebssType::THEAD_TEXT);
		}*/
		else if (*tagit == Tag::EXPAND)
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (!ent.getContent().isTheadStd())
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

void parseStdParamThead(Parser& parser, TheadStd& thead)
{
	auto webssThead = parser.parseThead(true);
	parseOtherValuesTheadStdAfterThead(parser, thead);
	auto& lastParam = thead.back();
	auto type = webssThead.getTypeRaw();
	switch (webssThead.getTypeRaw())
	{
	case TypeThead::SELF:
		lastParam.setThead(TheadSelf());
		break;
	case TypeThead::BIN:
		lastParam.setThead(move(webssThead.getTheadBinRaw()));
		break;
	case TypeThead::STD:
		lastParam.setThead(move(webssThead.getTheadStdRaw()));
		break;
	default:
		assert(false);
	}
}