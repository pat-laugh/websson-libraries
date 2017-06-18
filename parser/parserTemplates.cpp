//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

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

const char ERROR_EXPAND_BIN_TEMPLATE[] = "can't expand for a binary template";
const char ERROR_EXPAND_TYPE[] = "expanded item in template body must contain tuple to implement template head";

class ParserTemplates : public Parser
{
private:
	using ParamsStd = TheadStd::Params;
	using ParamsBin = TheadBin::Params;
public:
	Webss parseTemplateBin(Thead&& thead)
	{
		return parseTemplateBin(move(thead),
				[&](const ParamsBin& params) { return parseTemplateTupleBin(params); },
				[&](const ParamsBin& params) { return parseTemplateTupleBin(params); });
	}

	Webss parseTemplateStd(Thead&& thead)
	{
		if (thead.isText())
			return parseTemplateStd(thead,
				[&](const Thead& thead) { return parseTemplateTuple<true>(thead); },
				[&](const Thead& thead) { return parseTemplateTuple<true>(thead); });
		else
			return parseTemplateStd(thead,
				[&](const Thead& thead) { return parseTemplateTuple<false>(thead); },
				[&](const Thead& thead) { return parseTemplateTuple<true>(thead); });
	}

	Webss parseTemplateStd(const Thead& thead, function<Tuple(const Thead& thead)>&& funcTemplateTupleRegular, function<Tuple(const Thead& thead)>&& funcTemplateTupleText)
	{
		Tuple body;
		WebssType typeTuple;
		switch (tagit.getSafe())
		{
		case Tag::START_TUPLE:
			body = funcTemplateTupleRegular(thead);
			typeTuple = WebssType::TUPLE;
			break;
		case Tag::TEXT_TUPLE:
			body = funcTemplateTupleText(thead);
			typeTuple = WebssType::TUPLE_TEXT;
			break;
		default:
			if (!thead.isPlus())
				throw runtime_error("expected tuple");
			body = makeDefaultTuple(thead.getTheadStd().getParams());
			typeTuple = WebssType::TUPLE;
			break;
		}
		if (thead.isPlus())
			return Template(thead, move(body), parseValueOnly(), typeTuple);
		else
			return Template(thead, move(body), typeTuple);
	}

	Webss parseTemplateBin(Thead&& thead, function<Tuple(const ParamsBin& params)>&& funcTemplateTupleRegular, function<Tuple(const ParamsBin& params)>&& funcTemplateTupleText)
	{
		Tuple body;
		WebssType typeTuple;
		switch (tagit.getSafe())
		{
		case Tag::START_TUPLE:
			body = funcTemplateTupleRegular(thead.getTheadBin().getParams());
			typeTuple = WebssType::TUPLE;
			break;
		case Tag::TEXT_TUPLE:
			body = funcTemplateTupleText(thead.getTheadBin().getParams());
			typeTuple = WebssType::TUPLE_TEXT;
			break;
		default:
			if (!thead.isPlus())
				throw runtime_error("expected tuple");
			body = makeDefaultTuple(thead.getTheadStd().getParams());
			typeTuple = WebssType::TUPLE;
			break;
		}
		if (thead.isPlus())
			return Template(move(thead), move(body), parseValueOnly(), typeTuple);
		else
			return Template(move(thead), move(body), typeTuple);
	}

	template <bool isText>
	Tuple parseTemplateTuple(const Thead& thead)
	{
		const auto params = thead.getTheadStd().getParams();
		Tuple::size_type index = 0;
		Tuple tuple = parseContainer<Tuple, ConType::TUPLE>(Tuple(params.getSharedKeys()), true, [&](Tuple& tuple)
		{
			switch (*tagit)
			{
			case Tag::SEPARATOR: //void
				break;
			case Tag::EXPAND:
				index = expandTemplateTuple(params, tuple, index);
				return;
			case Tag::EXPLICIT_NAME:
			{
				auto name = parseNameExplicit(tagit);
				tuple.at(name) = isText ? Webss(parseValueOnly()) : parseTemplateContainer(thead, params.at(name));
				break;
			}
			case Tag::NAME_START:
				if (isText)
					tuple.at(index) = Webss(parseLineString(*this));
				else
					parseTemplateTupleName(thead, tuple, index);
				break;
			default:
				tuple.at(index) = isText ? Webss(parseLineString(*this)) : parseTemplateContainer(thead, params.at(index));
				break;
			}
			++index;
		});
		checkDefaultValues(tuple, params);
		return tuple;
	}

private:
	void expandTemplateList(const TheadBin::Params&, List&)
	{
		throw runtime_error(ERROR_EXPAND_BIN_TEMPLATE);
	}

	void expandTemplateList(const TheadStd::Params& params, List& list)
	{
		fillTemplateBodyList(params, parseExpandList(tagit, ents), list);
	}

	void parseTemplateTupleName(const Thead& thead, Tuple& tuple, Tuple::size_type& index)
	{
		const auto params = thead.getTheadStd().getParams();
		auto nameType = parseNameType(tagit, ents);
		if (nameType.type != NameType::NAME && params.at(index).hasThead())
			throw runtime_error(ERROR_UNEXPECTED);
		switch (nameType.type)
		{
		case NameType::NAME:
			tuple.at(nameType.name) = parseTemplateContainer(thead, params.at(nameType.name));
			break;
		case NameType::KEYWORD:
			tuple.at(index) = move(nameType.keyword);
			break;
		case NameType::ENTITY_ABSTRACT:
		{
			auto otherValue = checkAbstractEntity(nameType.entity);
			if (otherValue.type != OtherValue::VALUE_ONLY)
				throw runtime_error(ERROR_UNEXPECTED);
			tuple.at(index) = move(otherValue.value);
			break;
		}
		case NameType::ENTITY_CONCRETE:
			tuple.at(index) = move(nameType.entity);
			break;
		}
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

Webss Parser::parseTemplateContainer(const Thead& thead, const ParamStd& param)
{
	if (!param.hasThead())
		return parseValueOnly();
	switch (param.getTypeThead())
	{
	case TypeThead::SELF:
		return parseTemplateStd(thead);
	case TypeThead::BIN:
		return parseTemplateBin(param.getThead());
	case TypeThead::STD:
		return parseTemplateStd(param.getThead());
	default:
		assert(false); throw domain_error("");
	}
}

Tuple Parser::parseTemplateTupleStd(const Thead& thead)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateTuple<false>(thead);
}

Tuple Parser::parseTemplateTupleText(const Thead& thead)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateTuple<true>(thead);
}

Tuple::size_type Parser::expandTemplateTuple(const TheadStd::Params& params, Tuple& templateTuple, Tuple::size_type index)
{
	return fillTemplateBodyTuple(params, parseExpandTuple(tagit, ents), templateTuple, index);
}

List Parser::buildTemplateBodyList(const TheadStd::Params& params, const List& baseList)
{
	List templateList;
	fillTemplateBodyList(params, baseList, templateList);
	return templateList;
}

void Parser::fillTemplateBodyList(const TheadStd::Params& params, const List& baseList, List& filledList)
{
	for (const auto& item : baseList)
	{
		if (!item.isTuple())
			throw runtime_error(ERROR_EXPAND_TYPE);
		filledList.add(buildTemplateBodyTuple(params, item.getTuple()));
	}
}

Tuple Parser::buildTemplateBodyTuple(const TheadStd::Params& params, const Tuple& baseTuple)
{
	Tuple templateTuple(params.getSharedKeys());
	if (fillTemplateBodyTuple(params, baseTuple, templateTuple) < templateTuple.size())
		checkDefaultValues(templateTuple, params);
	return templateTuple;
}

Tuple::size_type Parser::fillTemplateBodyTuple(const TheadStd::Params& params, const Tuple& baseTuple, Tuple& filledTuple, Tuple::size_type index)
{
	for (const auto& item : baseTuple.getOrderedKeyValues())
	{
		if (item.first == nullptr)
			filledTuple.at(index) = checkTemplateContainer(params, params.at(index), *item.second);
		else
			filledTuple.at(*item.first) = checkTemplateContainer(params, params.at(*item.first), *item.second);
		++index;
	}
	return index;
}

Webss Parser::checkTemplateContainer(const TheadStd::Params& params, const ParamStd& param, const Webss& tupleItem)
{
	if (!param.hasThead())
		return tupleItem;
	switch (param.getTypeThead())
	{
	case TypeThead::SELF:
		return buildTemplateBodyStd(params, tupleItem);
	case TypeThead::BIN:
		throw runtime_error(ERROR_EXPAND_BIN_TEMPLATE);
	case TypeThead::STD:
		if (param.isPlusThead())
			; //...
		else
			return buildTemplateBodyStd(param.getTheadStd().getParams(), tupleItem);
	default:
		assert(false); throw domain_error("");
	}
}

Webss Parser::buildTemplateBodyStd(const TheadStd::Params& params, const Webss& templateItem)
{
	switch (templateItem.getType())
	{
	case WebssType::LIST:
		return buildTemplateBodyList(params, templateItem.getList());
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return buildTemplateBodyTuple(params, templateItem.getTuple());
	default:
		throw runtime_error("template head must be implemented");
	}
}

Webss Parser::parseTemplate()
{
	auto thead = parseThead();
	if (thead.isTheadBin())
		return parseTemplateBin(move(thead));
	else
	{
		assert(thead.isTheadStd());
		return parseTemplateStd(move(thead));
	}
}

Webss Parser::parseTemplateBin(Thead thead) { return static_cast<ParserTemplates*>(this)->parseTemplateBin(move(thead)); }
Webss Parser::parseTemplateStd(Thead thead) { return static_cast<ParserTemplates*>(this)->parseTemplateStd(move(thead)); }