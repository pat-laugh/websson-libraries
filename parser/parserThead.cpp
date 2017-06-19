//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserThead.hpp"

#include "containerSwitcher.hpp"
#include "utilsExpand.hpp"

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
	case Tag::START_TEMPLATE:
	default:
		return{ ParserThead::parseTheadStd(*this), options };
	case Tag::START_TUPLE:
		return{ ParserThead::parseTheadBin(*this), options };
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a template head must be within a non-empty template head");
		if (optionsSet)
			throw runtime_error("can't set options in a self template head");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TheadSelf();
	case Tag::START_LIST:
		if (optionsSet)
			throw runtime_error("template head options must be at the start only");
		options = ParserThead::parseTheadOptions(*this);
		if (!checkNextElement())
			return{ TheadStd(), options };
		optionsSet = true;
		goto switchStart;
	case Tag::EXPAND:
		break;
	}

	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::THEAD)
		throw runtime_error("expand entity within template head must be a template head");
	const auto& thead = ent.getContent().getThead();
	if (!optionsSet)
		options = thead.getOptions();
	if (!checkNextElement())
		return{ move(ent), options };
	assert(thead.isTheadBin() || thead.isTheadStd());
	if (thead.isTheadBin())
		return{ ParserThead::parseTheadBin(*this, TheadBin(thead.getTheadBin())), move(ent), options };
	else
		return{ ParserThead::parseTheadStd(*this, TheadStd(thead.getTheadStd())), move(ent), options };
}