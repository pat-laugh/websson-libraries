//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserThead.hpp"

#include "containerSwitcher.hpp"
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
		return{ ParserThead::parseTheadBin(*this), options };
	case Tag::SELF:
		if (!allowSelf)
			throw runtime_error("self in a template head must be within a non-empty template head");
		if (optionsSet)
			throw runtime_error("can't set options in a self template head");
		(++tagit).sofertTag(Tag::END_TEMPLATE);
		++tagit;
		return TheadSelf();
	case Tag::START_TEMPLATE_FUNCTION:
	startTemplateFunction:
	{
		Thead thead(parseTheadFunction(), options);
		if (checkNextElement())
			throw runtime_error(ERROR_UNEXPECTED);
		return thead;
	}
	//options
	case Tag::PLUS:
		options.isPlus = true;
		++tagit;
		if (!checkNextElement())
			return{ TheadStd(), options };
		optionsSet = true;
		goto switchStart;
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
		return{ ParserThead::parseTheadBin(*this, thead.getTheadBin().makeCompleteCopy()), move(ent), options };
	if (*tagit != Tag::START_TUPLE)
		return{ ParserThead::parseTheadStd(*this, thead.getTheadStd().makeCompleteCopy()), move(ent), options };

	Tuple modifierTuple;
	auto theadCopy = thead.getTheadStd().makeCompleteCopy();
	auto& params = const_cast<BasicParams<ParamStd>&>(theadCopy.getParams());
	auto tuple = Parser::ParserTempl::parseTemplateTuple<false, false>(*this, theadCopy);
	for (const auto& item : tuple.getOrderedKeyValues())
		if (!item.second->isNone())
		{
			params[*item.first] = *item.second;
			modifierTuple.add(*item.first, move(*item.second));
		}
	if (!checkNextElement())
		return{ move(theadCopy), move(ent), options, move(modifierTuple) };
	return{ ParserThead::parseTheadStd(*this, move(theadCopy)), move(ent), options, move(modifierTuple) };
}

TheadFun Parser::parseTheadFunction()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_FUNCTION, false);
	TheadFun theadFun;
	
	if (!containerEmpty())
		theadFun.setThead(ParserThead::parseTheadStd(*this));
	
	int index = 0;
	for (string* name : theadFun.getThead().getParams().getOrderedKeys())
		ents.addPublicSafe(*name, Placeholder(index++, theadFun.getPointerRaw()));
	
	theadFun.setStructure(parseValueOnly());
	
	for (string* name : theadFun.getThead().getParams().getOrderedKeys())
		ents.removePublic(*name);
	
	return theadFun;
}