//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserThead.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utils/constants.hpp"

using namespace std;
using namespace webss;

Thead Parser::parseThead(bool allowSelf)
{
	ContainerSwitcher switcher(*this, ConType::THEAD, false);
	if (containerEmpty())
		return TheadStd();

	bool optionsSet = false;
	TheadOptions options;
switchStart:
	switch (*tagit)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
	default:
		return{ ParserThead::parseTheadStd(*this), options };
	case Tag::START_TUPLE:
		return{ ParserThead::parseTheadBin(*this), options };
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a thead must be within a non-empty thead");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return{ TheadSelf(), options };
	case Tag::START_LIST:
		if (optionsSet)
			throw runtime_error("template head options must be at the start only");
		options = ParserThead::parseTheadOptions(*this);
		optionsSet = true;
		goto switchStart;
	case Tag::EXPAND:
		break;
	}

	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::THEAD)
		throw runtime_error("expand entity in template head must be a template head");
	const auto& thead = ent.getContent().getThead();
	if (!optionsSet)
		options = thead.getOptions();
	if (!checkNextElement())
		return{ ent, options };
	auto typeThead = thead.getType();
	assert(typeThead == TypeThead::BIN || typeThead == TypeThead::STD);
	if (typeThead == TypeThead::BIN)
		return{ ParserThead::parseTheadBin(*this, TheadBin(thead.getTheadBin())), ent, options };
	else
		return{ ParserThead::parseTheadStd(*this, TheadStd(thead.getTheadStd())), ent, options };
}