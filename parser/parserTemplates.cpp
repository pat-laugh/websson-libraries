//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parserStrings.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";
const char ERROR_EXPAND_BINARY_TEMPLATE[] = "can't expand for a binary template";
const char ERROR_EXPAND_TYPE[] = "expanded item in template body must contain tuple to implement template head";


void setDefaultValue(Webss& value, const ParamBinary& defaultValue);
void setDefaultValue(Webss& value, const ParamStandard& defaultValue);

template <class Parameters>
Tuple makeDefaultTuple(const Parameters& params)
{
	Tuple tuple(params.getSharedKeys());
	for (Tuple::size_type i = 0; i < params.size(); ++i)
		setDefaultValue(tuple[i], params[i]);
	return tuple;
}

void setDefaultValue(Webss& value, const ParamBinary& defaultValue)
{
	if (defaultValue.hasDefaultValue())
		value = Webss(defaultValue.getDefaultPointer());
	else if (defaultValue.isTemplateHeadBinary())
		value = makeDefaultTuple(defaultValue.getTemplateHead().getParameters());
	else
		throw runtime_error(ERROR_NO_DEFAULT);
}

void setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasDefaultValue())
		value = Webss(defaultValue.getDefaultPointer());
	else
	{
		switch (defaultValue.getTypeThead())
		{
		case WebssType::TEMPLATE_HEAD_BINARY:
			value = makeDefaultTuple(defaultValue.getTemplateHeadBinary().getParameters());
			break;
		case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
			value = makeDefaultTuple(defaultValue.getTemplateHeadStandard().getParameters());
			break;
		default:
			throw runtime_error(ERROR_NO_DEFAULT);
		}
	}
}

void checkDefaultValues(Tuple& tuple, const TemplateHeadStandard::Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}

class ParserTemplates : public Parser
{
private:
	using ParametersStandard = TemplateHeadStandard::Parameters;
	using ParametersBinary = TemplateHeadBinary::Parameters;
public:
	Webss parseTemplateBodyStandard(const ParametersStandard& params)
	{
		return parseTemplateBodyStandard(params, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<false>(params)); }, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), WebssType::TUPLE_TEXT); });
	}

	Webss parseTemplateBodyText(const ParametersStandard& params)
	{
		return parseTemplateBodyStandard(params, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), WebssType::TUPLE_TEXT); }, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), WebssType::TUPLE_TEXT); });
	}

	Webss parseTemplateBodyStandard(const ParametersStandard& params, function<Webss(const ParametersStandard& params)>&& funcTemplTupleRegular, function<Webss(const ParametersStandard& params)>&& funcTemplTupleText)
	{
		switch (nextTag = getTag(it))
		{
		case Tag::START_DICTIONARY:
			return parseTemplateDictionary<ParametersStandard>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_LIST:
			return parseTemplateList<ParametersStandard>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_TUPLE:
			return funcTemplTupleRegular(params);
		case Tag::TEXT_TUPLE:
			return funcTemplTupleText(params);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

	Webss parseTemplateBodyBinary(const ParametersBinary& params, function<Webss(const ParametersBinary& params)>&& funcTemplTupleRegular, function<Webss(const ParametersBinary& params)>&& funcTemplTupleText)
	{
		switch (nextTag = getTag(it))
		{
		case Tag::START_DICTIONARY:
			return parseTemplateDictionary<ParametersBinary>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_LIST:
			return parseTemplateList<ParametersBinary>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_TUPLE:
			return funcTemplTupleRegular(params);
		case Tag::TEXT_TUPLE:
			return funcTemplTupleText(params);
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
			switch (nextTag)
			{
			case Tag::SEPARATOR: //void
				break;
			case Tag::EXPAND:
				index = expandTemplateTuple(params, tuple, index);
				return;
			case Tag::EXPLICIT_NAME:
			{
				auto name = parseNameExplicit(it);
				nextTag = getTag(it);
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
	void expandTemplateDictionary(const TemplateHeadBinary::Parameters&, Dictionary&)
	{
		throw runtime_error(ERROR_EXPAND_BINARY_TEMPLATE);
	}

	void expandTemplateDictionary(const TemplateHeadStandard::Parameters& params, Dictionary& dict)
	{
		fillTemplateBodyDictionary(params, parseExpandDictionary(it, ents), dict);
	}

	void expandTemplateList(const TemplateHeadBinary::Parameters&, List&)
	{
		throw runtime_error(ERROR_EXPAND_BINARY_TEMPLATE);
	}

	void expandTemplateList(const TemplateHeadStandard::Parameters& params, List& list)
	{
		fillTemplateBodyList(params, parseExpandList(it, ents), list);
	}

	void parseTemplateTupleName(const TemplateHeadStandard::Parameters& params, Tuple& tuple, Tuple::size_type& index)
	{
		auto nameType = parseNameType(it, ents);
		if (nameType.type != NameType::NAME && params.at(index).hasTemplateHead())
			throw runtime_error(ERROR_UNEXPECTED);
		switch (nameType.type)
		{
		case NameType::NAME:
			nextTag = getTag(it);
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

	string parseNameTemplateDictionary()
	{
		if (nextTag == Tag::NAME_START)
			return parseName(it);
		else if (nextTag == Tag::EXPLICIT_NAME)
			return parseNameExplicit(it);
		throw runtime_error("dictionary can only have key-values");
	}

	template <class Parameters>
	Dictionary parseTemplateDictionary(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), false, [&](Dictionary& dict)
		{
			if (nextTag == Tag::EXPAND)
				expandTemplateDictionary(params, dict);
			else
			{
				string name = parseNameTemplateDictionary();
				switch (nextTag = getTag(it))
				{
				case Tag::START_LIST:
					dict.addSafe(move(name), parseTemplateList<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText)));
					break;
				case Tag::START_TUPLE:
					dict.addSafe(move(name), funcTemplTupleRegular(params));
					break;
				case Tag::TEXT_TUPLE:
					dict.addSafe(move(name), funcTemplTupleText(params));
					break;
				default:
					throw runtime_error(ERROR_UNEXPECTED);
				}
			}
		});
	}

	template <class Parameters>
	List parseTemplateList(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<List, ConType::LIST>(List(), false, [&](List& list)
		{
			switch (nextTag)
			{
			case Tag::EXPAND:
				expandTemplateList(params, list);
				break;
			case Tag::START_TUPLE:
				list.add(funcTemplTupleRegular(params));
				break;
			case Tag::TEXT_TUPLE:
				list.add(funcTemplTupleText(params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		});
	}
};

Webss Parser::parseTemplate()
{
	auto headWebss = parseTemplateHead();
	switch (headWebss.getTypeRaw())
	{
	case WebssType::BLOCK_HEAD:
		nextTag = getTag(it);
		return Block(move(headWebss.getBlockHeadRaw()), parseValueOnly());
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		auto head = move(headWebss.getTemplateHeadBinaryRaw());
		auto body = parseTemplateBodyBinary(head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SELF:
		throw runtime_error("self in a thead must be within a non-empty thead");
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		auto head = move(headWebss.getTemplateHeadStandardRaw());
		auto body = parseTemplateBodyStandard(head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_TEXT:
	{
		auto head = move(headWebss.getTemplateHeadStandardRaw());
		auto body = parseTemplateBodyText(head.getParameters());
		return{ move(head), move(body), WebssType::TEMPLATE_TEXT };
	}
#ifdef assert
	default:
		assert(false);
		throw domain_error("");
#endif
	}
}

Webss Parser::parseTemplateText()
{
	auto head = parseTemplateHeadText();
	auto body = parseTemplateBodyText(head.getParameters());
	return{ move(head), move(body), WebssType::TEMPLATE_TEXT };
}

Webss Parser::parseTemplateBodyBinary(const TemplateHeadBinary::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyBinary(params, [&](const TemplateHeadBinary::Parameters& params) { return parseTemplateTupleBinary(params); }, [&](const TemplateHeadBinary::Parameters&) -> Webss { throw runtime_error(ERROR_UNEXPECTED); });
}

Webss Parser::parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyStandard(params);
}

Webss Parser::parseTemplateBodyText(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyText(params);
}

Webss Parser::parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue)
{
	switch (defaultValue.getTypeThead())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		return parseTemplateBodyBinary(defaultValue.getTemplateHeadBinary().getParameters());
	case WebssType::TEMPLATE_HEAD_SELF:
		return parseTemplateBodyStandard(params);
	case WebssType::TEMPLATE_HEAD_STANDARD:
		return parseTemplateBodyStandard(defaultValue.getTemplateHeadStandard().getParameters());
	case WebssType::TEMPLATE_HEAD_TEXT:
		return parseTemplateBodyText(defaultValue.getTemplateHeadStandard().getParameters());
	default:
		return parseValueOnly();
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
	return fillTemplateBodyTuple(params, parseExpandTuple(it, ents), templateTuple, index);
}

Dictionary Parser::buildTemplateBodyDictionary(const TemplateHeadStandard::Parameters& params, const Dictionary& baseDictionary)
{
	Dictionary templateDictionary;
	fillTemplateBodyDictionary(params, baseDictionary, templateDictionary);
	return templateDictionary;
}

void Parser::fillTemplateBodyDictionary(const TemplateHeadStandard::Parameters& params, const Dictionary& baseDictionary, Dictionary& filledDictionary)
{
	for (const auto& item : baseDictionary)
	{
		if (item.second.isTuple())
			filledDictionary.addSafe(item.first, buildTemplateBodyTuple(params, item.second.getTuple()));
		else if (item.second.isList())
			filledDictionary.addSafe(item.first, buildTemplateBodyList(params, item.second.getList()));
		else
			throw runtime_error(ERROR_EXPAND_TYPE);
	}
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
	switch (param.getTypeThead())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		throw runtime_error(ERROR_EXPAND_BINARY_TEMPLATE);
	case WebssType::TEMPLATE_HEAD_SELF:
		return buildTemplateBodyStandard(params, tupleItem);
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		return buildTemplateBodyStandard(param.getTemplateHeadStandard().getParameters(), tupleItem);
	default:
		return tupleItem;
	}
}

Webss Parser::buildTemplateBodyStandard(const TemplateHeadStandard::Parameters& params, const Webss& templateItem)
{
	switch (templateItem.getType())
	{
	case WebssType::DICTIONARY:
		return buildTemplateBodyDictionary(params, templateItem.getDictionary());
	case WebssType::LIST: case WebssType::LIST_TEXT:
		return buildTemplateBodyList(params, templateItem.getList());
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return buildTemplateBodyTuple(params, templateItem.getTuple());
	default:
		throw runtime_error("template head must be implemented");
	}
}