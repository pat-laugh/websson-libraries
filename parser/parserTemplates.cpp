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
	return parseTemplateBody(move(thead));
}

Webss Parser::parseTemplateBody(Thead thead)
{
	//theadFun body is parsed exactly like theadStd body
	assert(thead.isTheadBin() || thead.isTheadStd() || thead.isTheadFun());
	const function<Webss(Parser&, Thead)>& funcTempl = thead.isTheadBin() ? ParserTempl::parseTemplateBin : ParserTempl::parseTemplateStd;
	if (tagit.getSafe() != Tag::FOREACH)
		return funcTempl(*this, move(thead));
	return parseTemplateForeach(thead, funcTempl);
}

List Parser::parseTemplateForeach(const Thead& thead, const function<Webss(Parser&, Thead)>& funcTempl)
{
	if (*++tagit != Tag::START_LIST)
	{
		List list;
		ParserTempl::foreachValue(*this, thead, parseValueOnly(), list);
		return list;
	}
	
	//optimization
	return parseContainer<List, ConType::LIST>(List(), true, [&](List& list)
	{
		switch (*tagit)
		{
		case Tag::SEPARATOR: //void
			if (thead.isPlus())
				throw runtime_error(ERROR_VOID);
			list.add(Template(thead, makeDefaultTuple(thead)));
			break;
		case Tag::EXPAND:
			if (thead.isTheadBin())
				throw runtime_error("can't use expand with binary template");
			ParserTempl::foreachList(*this, thead, parseExpandList(tagit, ents), list);
			break;
		case Tag::EXPLICIT_NAME:
			throw runtime_error("list can only contain values");
		case Tag::FOREACH:
			list.add(parseTemplateForeach(thead, funcTempl));
			break;
		default:
			list.add(funcTempl(*this, thead));
			break;
		}
	});
}