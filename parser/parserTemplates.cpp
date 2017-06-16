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

const char ERROR_EXPAND_BINARY_TEMPLATE[] = "can't expand for a binary template";
const char ERROR_EXPAND_TYPE[] = "expanded item in template body must contain tuple to implement template head";

class ParserTemplates : public Parser
{
private:
	using ParametersStandard = TemplateHeadStandard::Parameters;
	using ParametersBinary = TemplateHeadBinary::Parameters;
public:
	Webss parseTemplateBinary(TemplateHeadBinary&& thead)
	{
		return parseTemplateBinary(move(thead),
				[&](const ParametersBinary& params) { return parseTemplateTupleBinary(params); },
				[&](const ParametersBinary& params) { return parseTemplateTupleBinary(params); });
	}

	Webss parseTemplateStandard(TemplateHeadStandard&& thead)
	{
		return parseTemplateStandard<WebssType::TEMPLATE_STANDARD>(move(thead),
				[&](const ParametersStandard& params) { return parseTemplateTuple<false>(params); },
				[&](const ParametersStandard& params) { return parseTemplateTuple<true>(params); });
	}

	Webss parseTemplateText(TemplateHeadStandard&& thead)
	{
		return parseTemplateStandard<WebssType::TEMPLATE_TEXT>(move(thead),
				[&](const ParametersStandard& params) { return parseTemplateTuple<true>(params); },
				[&](const ParametersStandard& params) { return parseTemplateTuple<true>(params); });
	}

	template <WebssType::Enum type>
	Webss parseTemplateStandard(TemplateHeadStandard&& thead, function<Tuple(const ParametersStandard& params)>&& funcTemplateTupleRegular, function<Tuple(const ParametersStandard& params)>&& funcTemplateTupleText)
	{
		Tuple body;
		switch (tagit.getSafe())
		{
	//	case Tag::START_LIST:
	//		return parseTemplateList<ParametersStandard>(params, move(funcTemplateTupleRegular), move(funcTemplateTupleText));
		case Tag::START_TUPLE:
			body = funcTemplateTupleRegular(thead.getParameters());
			return{ TemplateStandard(move(thead), move(body), WebssType::TUPLE), type };
		case Tag::TEXT_TUPLE:
			body = funcTemplateTupleText(thead.getParameters());
			return{ TemplateStandard(move(thead), move(body), WebssType::TUPLE_TEXT), type };
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

	Webss parseTemplateBinary(TemplateHeadBinary&& thead, function<Tuple(const ParametersBinary& params)>&& funcTemplateTupleRegular, function<Tuple(const ParametersBinary& params)>&& funcTemplateTupleText)
	{
		Tuple body;
		switch (tagit.getSafe())
		{
	//	case Tag::START_LIST:
	//		return parseTemplateList<ParametersBinary>(params, move(funcTemplateTupleRegular), move(funcTemplateTupleText));
		case Tag::START_TUPLE:
			body = funcTemplateTupleRegular(thead.getParameters());
			return TemplateBinary(move(thead), move(body), WebssType::TUPLE);
		case Tag::TEXT_TUPLE:
			body = funcTemplateTupleText(thead.getParameters());
			return TemplateBinary(move(thead), move(body), WebssType::TUPLE_TEXT);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

	template <bool isText>
	Tuple parseTemplateTuple(const TemplateHeadStandard::Parameters& params)
	{
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
				tuple.at(name) = isText ? Webss(parseValueOnly()) : parseTemplateContainer(params, params.at(name));
				break;
			}
			case Tag::NAME_START:
				if (isText)
					tuple.at(index) = Webss(parseLineString(*this));
				else
					parseTemplateTupleName(params, tuple, index);
				break;
			default:
				tuple.at(index) = isText ? Webss(parseLineString(*this)) : parseTemplateContainer(params, params.at(index));
				break;
			}
			++index;
		});
		checkDefaultValues(tuple, params);
		return tuple;
	}

private:
	void expandTemplateList(const TemplateHeadBinary::Parameters&, List&)
	{
		throw runtime_error(ERROR_EXPAND_BINARY_TEMPLATE);
	}

	void expandTemplateList(const TemplateHeadStandard::Parameters& params, List& list)
	{
		fillTemplateBodyList(params, parseExpandList(tagit, ents), list);
	}

	void parseTemplateTupleName(const TemplateHeadStandard::Parameters& params, Tuple& tuple, Tuple::size_type& index)
	{
		auto nameType = parseNameType(tagit, ents);
		if (nameType.type != NameType::NAME && params.at(index).hasThead())
			throw runtime_error(ERROR_UNEXPECTED);
		switch (nameType.type)
		{
		case NameType::NAME:
			tuple.at(nameType.name) = parseTemplateContainer(params, params.at(nameType.name));
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

	template <class Parameters>
	List parseTemplateList(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplateTupleRegular, function<Webss(const Parameters& params)>&& funcTemplateTupleText)
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
/*
Webss Parser::parseTemplateText()
{
	auto head = parseTemplateHeadText();
	auto body = parseTemplateText(head.getParameters());
	return{ move(head), move(body), WebssType::TEMPLATE_TEXT };
}*/

Webss Parser::parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& param)
{
	if (!param.hasThead())
		return parseValueOnly();
	switch (param.getTypeThead())
	{
	case TypeThead::SELF:
		return parseTemplateStandard(params);
	case TypeThead::BINARY:
		if (param.getThead().isPlus())
			return parseTemplatePlusBinary(param.getTheadBin().getParameters());
		else
			return parseTemplateBinary(param.getTheadBin().getParameters());
	case TypeThead::STANDARD:
		if (param.getThead().isPlus())
		{
			if (param.getThead().isText())
				return parseTemplatePlusText(param.getTheadStd().getParameters());
			else
				return parseTemplatePlusStandard(param.getTheadStd().getParameters());
		}
		else
		{
			if (param.getThead().isText())
				return parseTemplateStandard(param.getTheadStd().getParameters());
			else
				return parseTemplateText(param.getTheadStd().getParameters());
		}
	default:
		assert(false); throw domain_error("");
	}
}

Tuple Parser::parseTemplateTupleStandard(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateTuple<false>(params);
}

Tuple Parser::parseTemplateTupleText(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateTuple<true>(params);
}

Tuple::size_type Parser::expandTemplateTuple(const TemplateHeadStandard::Parameters& params, Tuple& templateTuple, Tuple::size_type index)
{
	return fillTemplateBodyTuple(params, parseExpandTuple(tagit, ents), templateTuple, index);
}

List Parser::buildTemplateBodyList(const TemplateHeadStandard::Parameters& params, const List& baseList)
{
	List templateList;
	fillTemplateBodyList(params, baseList, templateList);
	return templateList;
}

void Parser::fillTemplateBodyList(const TemplateHeadStandard::Parameters& params, const List& baseList, List& filledList)
{
	for (const auto& item : baseList)
	{
		if (!item.isTuple())
			throw runtime_error(ERROR_EXPAND_TYPE);
		filledList.add(buildTemplateBodyTuple(params, item.getTuple()));
	}
}

Tuple Parser::buildTemplateBodyTuple(const TemplateHeadStandard::Parameters& params, const Tuple& baseTuple)
{
	Tuple templateTuple(params.getSharedKeys());
	if (fillTemplateBodyTuple(params, baseTuple, templateTuple) < templateTuple.size())
		checkDefaultValues(templateTuple, params);
	return templateTuple;
}

Tuple::size_type Parser::fillTemplateBodyTuple(const TemplateHeadStandard::Parameters& params, const Tuple& baseTuple, Tuple& filledTuple, Tuple::size_type index)
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

Webss Parser::checkTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& param, const Webss& tupleItem)
{
	if (!param.hasThead())
		return tupleItem;
	switch (param.getTypeThead())
	{
	case TypeThead::SELF:
		return buildTemplateBodyStandard(params, tupleItem);
	case TypeThead::BINARY:
		throw runtime_error(ERROR_EXPAND_BINARY_TEMPLATE);
	case TypeThead::STANDARD:
		if (param.isPlusThead())
			; //...
		else
			return buildTemplateBodyStandard(param.getTheadStd().getParameters(), tupleItem);
	default:
		assert(false); throw domain_error("");
	}
}

Webss Parser::buildTemplateBodyStandard(const TemplateHeadStandard::Parameters& params, const Webss& templateItem)
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
	auto headWebss = parseThead();
	switch (headWebss.getTypeRaw())
	{
	case TypeThead::BINARY:
		return parseTemplateBinary(headWebss.getTheadBinaryRaw());
	case TypeThead::STANDARD:
		return parseTemplateStandard(headWebss.getTheadStandardRaw());
/*	case WebssType::TEMPLATE_HEAD_TEXT:
		return parseTemplateText(headWebss.getTheadStandardRaw());
	case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		return parseTemplatePlusBinary(headWebss.getTemplateHeadBinaryRaw());
	case WebssType::TEMPLATE_HEAD_PLUS_STANDARD:
		return parseTemplatePlusStandard(headWebss.getTemplateHeadStandardRaw());
	case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		return parseTemplatePlusText(headWebss.getTemplateHeadStandardRaw());*/
	default:
		assert(false); throw domain_error("");
	}
}

Webss Parser::parseTemplateBinary(TemplateHeadBinary thead) { return static_cast<ParserTemplates*>(this)->parseTemplateBinary(move(thead)); }
Webss Parser::parseTemplateStandard(TemplateHeadStandard thead) { return static_cast<ParserTemplates*>(this)->parseTemplateStandard(move(thead)); }
Webss Parser::parseTemplateText(TemplateHeadStandard thead) { return static_cast<ParserTemplates*>(this)->parseTemplateText(move(thead)); }

Webss Parser::parseTemplatePlusBinary(TemplateHeadBinary thead)
{
	Tuple body;
	switch (tagit.getSafe())
	{
	case Tag::START_TUPLE: case Tag::TEXT_TUPLE:
		body = parseTemplateTupleBinary(thead.getParameters());
		break;
	default:
		body = makeDefaultTuple(thead.getParameters());
		break;
	}
	return TemplatePlusBinary(TemplateBinary(move(thead), move(body)), parseValueOnly());
}

Webss Parser::parseTemplatePlusStandard(TemplateHeadStandard thead)
{
	Tuple body;
	WebssType tupleType;
	switch (tagit.getSafe())
	{
	case Tag::START_TUPLE:
		tupleType = WebssType::TUPLE;
		body = parseTemplateTupleStandard(thead.getParameters());
		break;
	case Tag::TEXT_TUPLE:
		tupleType = WebssType::TUPLE_TEXT;
		body = parseTemplateTupleText(thead.getParameters());
		break;
	default:
		tupleType = WebssType::TUPLE;
		body = makeDefaultTuple(thead.getParameters());
		break;
	}
	return TemplatePlusStandard(TemplateStandard(move(thead), move(body), tupleType), parseValueOnly());
}

Webss Parser::parseTemplatePlusText(TemplateHeadStandard thead)
{
	Tuple body;
	WebssType tupleType;
	switch (tagit.getSafe())
	{
	case Tag::START_TUPLE: case Tag::TEXT_TUPLE:
		body = parseTemplateTupleText(thead.getParameters());
		break;
	default:
		body = makeDefaultTuple(thead.getParameters());
		break;
	}
	return{ TemplatePlusStandard(TemplateStandard(move(thead), move(body)), parseValueOnly()), WebssType::TEMPLATE_PLUS_TEXT };
}