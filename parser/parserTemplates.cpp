//MIT License
//Copyright 2017 Patrick Laughrea
#include "parserTempl.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parserStrings.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utilsTemplateDefaultValues.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Webss Parser::parseTemplate()
{
	auto thead = parseThead();
	assert(thead.isTheadBin() || thead.isTheadStd());
	if (thead.isTheadBin())
		return parseTemplateBin(move(thead));
	else
		return parseTemplateStd(move(thead));
}

Webss Parser::parseTemplateBin(Thead thead)
{
	Tuple body;
	auto tag = tagit.getSafe();
	if (tag == Tag::START_TUPLE || tag == Tag::TEXT_TUPLE)
		body = parseTemplateTupleBin(thead.getTheadBin().getParams());
	else if (thead.isPlus())
		body = makeDefaultTuple(thead.getTheadBin().getParams());
	else
		throw runtime_error("expected tuple");

	if (thead.isPlus())
		return Template(move(thead), move(body), parseValueOnly());
	else
		return Template(move(thead), move(body));
}

#define ParseTemplateTuple(IsText) ParserTempl::parseTemplateTuple<IsText>(*this, thead)

Webss Parser::parseTemplateStd(Thead thead)
{
	Tuple body;
	WebssType typeTuple = WebssType::TUPLE;
	auto tag = tagit.getSafe();
	if (tag == Tag::START_TUPLE)
		body = thead.isText() ? ParseTemplateTuple(true) : ParseTemplateTuple(false);
	else if (tag == Tag::TEXT_TUPLE)
	{
		body = ParseTemplateTuple(true);
		typeTuple = WebssType::TUPLE_TEXT;
	}
	else if (thead.isPlus())
		body = makeDefaultTuple(thead.getTheadStd().getParams());
	else
		throw runtime_error("expected tuple");

	if (thead.isPlus())
		return Template(move(thead), move(body), parseValueOnly(), typeTuple);
	else
		return Template(move(thead), move(body), typeTuple);
}

/*
class ParserTemplates : public Parser
{
private:
	void expandTemplateList(const TheadBin::Params&, List&)
	{
		throw runtime_error(ERROR_EXPAND_BIN_TEMPLATE);
	}

	void expandTemplateList(const TheadStd::Params& params, List& list)
	{
		fillTemplateBodyList(params, parseExpandList(tagit, ents), list);
	}

	template <class Params>
	List parseTemplateList(const Params& params, function<Webss(const Params& params)>&& funcTemplateTupleRegular, function<Webss(const Params& params)>&& funcTemplateTupleText)
	{
		return parseContainer<List, ConType::LIST>(List(), true, [&](List& list)
		{
			switch (*tagit)
			{
			case Tag::SEPARATOR: //void
				list.add(makeDefaultTuple(params));
				break;
			case Tag::EXPAND:
				expandTemplateList(params, list);
				break;
			case Tag::START_TUPLE:
				list.add(funcTemplateTupleRegular(params));
				break;
			case Tag::TEXT_TUPLE:
				list.add(funcTemplateTupleText(params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		});
	}
};
*/