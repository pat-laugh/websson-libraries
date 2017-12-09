//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserThead.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "utilsExpand.hpp"
#include "parserTempl.hpp"
#include "structures/theadFun.hpp"
#include "structures/placeholder.hpp"

using namespace std;
using namespace webss;

Thead Parser::parseThead(bool allowSelf)
{
	assert(CHAR_START_TEMPLATE_BIN == CHAR_START_DICTIONARY); //required to parse correctly template option text with binary template
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
	case Tag::START_TEMPLATE_BIN:
	startTemplateBin:
	{
		TheadBin thead;
		{
			ContainerSwitcher switcherBin(*this, ConType::TEMPLATE_BIN, false);
			thead = containerEmpty() ? TheadBin() : parseTheadBin();
		}
		if (checkNextElement())
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
		return{ move(thead), options };
	}
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error(WEBSSON_EXCEPTION("self in a template head must be within a non-empty template head"));
		if (optionsSet)
			throw runtime_error(WEBSSON_EXCEPTION("can't set options in a self template head"));
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TheadSelf();
	case Tag::START_TEMPLATE_FUN:
	startTemplateFunction:
	{
		auto theadFun = parseTheadFun();
		if (checkNextElement())
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
		parseTheadFunStructure(theadFun);
		return Thead(move(theadFun), options);
	}
	//options
	optionSwitchStart:
		if (!checkNextElement())
			return{ TheadStd(), options };
		optionsSet = true;
		goto switchStart;
	case Tag::MINUS:
		options.isText = false;
		options.isPlus = false;
		++tagit;
		goto optionSwitchStart;
	case Tag::PLUS:
		options.isPlus = true;
		++tagit;
		goto optionSwitchStart;
	//for tag text, the parser reads as if :: was going to lead to a container
	case Tag::TEXT_TUPLE: //as if <::(...)> for text template function
		options.isText = true;
		tagit.update();
		goto startTemplateFunction;
	case Tag::TEXT_DICTIONARY: //this can be <::{...}> (binary template) or anything else (just the option)
		options.isText = true;
		tagit.update();
		if (*tagit.getIt() == CHAR_START_DICTIONARY)
			goto startTemplateBin;
		goto optionSwitchStart;
	case Tag::EXPAND:
		break;
	}

	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::THEAD)
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within template head must be a template head"));
	const auto& thead = ent.getContent().getThead();
	if (!optionsSet)
		options = thead.getOptions();
	if (!checkNextElement())
		return{ move(ent), options };
	assert(thead.isTheadBin() || thead.isTheadStd() || thead.isTheadFun());
	if (thead.isTheadBin())
		return{ parseTheadBin(thead.getTheadBin().makeCompleteCopy()), move(ent), options };
	auto theadCopy = (thead.isTheadStd() ? thead.getTheadStd() : thead.getTheadFun().getThead()).makeCompleteCopy();
	if (*tagit != Tag::START_TUPLE)
	{
		theadCopy = ParserThead::parseTheadStd(*this, move(theadCopy));
		if (thead.isTheadStd())
			return{ move(theadCopy), move(ent), options };
		else
		{
			TheadFun theadFun;
			theadFun.setThead(move(theadCopy));
			parseTheadFunStructure(theadFun);
			return{ move(theadFun), move(ent), options };
		}
	}

	Tuple modifierTuple;
	auto& params = const_cast<BasicParams<ParamStd>&>(theadCopy.getParams());
	auto tuple = Parser::ParserTempl::parseTemplateTuple<false, false>(*this, theadCopy);
	for (const auto& item : tuple.getOrderedKeyValues())
		if (!item.second->isNone())
		{
			params[*item.first] = *item.second;
			modifierTuple.add(*item.first, move(*item.second));
		}
	if (thead.isTheadStd())
	{
		if (!checkNextElement())
			return{ move(theadCopy), move(ent), options, move(modifierTuple) };
		return{ ParserThead::parseTheadStd(*this, move(theadCopy)), move(ent), options, move(modifierTuple) };
	}
	else
	{
		TheadFun theadFun;
		theadFun.setThead(!checkNextElement() ? move(theadCopy) : ParserThead::parseTheadStd(*this, move(theadCopy)));
		parseTheadFunStructure(theadFun);
		return{ move(theadFun), move(ent), options, move(modifierTuple) };
	}
}

TheadBin Parser::parseTheadBin(TheadBin&& thead)
{
	do
		parseBinHead(thead);
	while (checkNextElement());
	return thead;
}

TheadFun Parser::parseTheadFun()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_FUN, false);
	TheadFun theadFun;
	theadFun.setThead(containerEmpty() ? TheadStd() : ParserThead::parseTheadStd(*this));
	return theadFun;
}

void Parser::parseTheadFunStructure(TheadFun& theadFun)
{
	int index = 0;
	for (string* name : theadFun.getThead().getParams().getOrderedKeys())
		ents.addSafe(*name, Placeholder(index++, theadFun.getPointerRaw()));
	
	theadFun.setStructure(parseValueOnly());
	
	for (string* name : theadFun.getThead().getParams().getOrderedKeys())
		ents.remove(*name);
}