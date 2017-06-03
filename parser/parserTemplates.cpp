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
		return parseTemplateBodyStandard(params, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<false>(params)); }, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), true); });
	}

	Webss parseTemplateBodyText(const ParametersStandard& params)
	{
		return parseTemplateBodyStandard(params, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), true); }, [&](const ParametersStandard& params) { return Webss(parseTemplateTuple<true>(params), true); });
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
	template <class Parameters>
	void expandTemplateDictionary(const Parameters& params, Dictionary& dict)
	{
		auto ent = parseExpandEntity(it, ents);
		if (ent.getContent().getType() != WebssType::DICTIONARY)
			throw runtime_error("expand entity within dictionary must be a dictionary");
		for (const auto& item : ent.getContent().getDictionary())
			dict.addSafe(item.first, item.second);
	}

	template <class Parameters>
	void expandTemplateList(const Parameters& params, List& list)
	{
		auto ent = parseExpandEntity(it, ents);
		if (ent.getContent().getType() != WebssType::LIST && ent.getContent().getType() != WebssType::LIST_TEXT)
			throw runtime_error("expand entity within list must be a list");
		for (const auto& item : ent.getContent().getList())
			list.add(item);
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
		return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), true, [&](Dictionary& dict)
		{
			if (nextTag == Tag::EXPAND)
				expandTemplateDictionary(params, dict);
			else if (nextTag == Tag::SEPARATOR)
				; //...
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
		return parseContainer<List, ConType::LIST>(List(), true, [&](List& list)
		{
			switch (nextTag)
			{
			case Tag::SEPARATOR:
				//...
				break;
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
		return{ move(head), move(body), true };
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
	return{ move(head), move(body), true };
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
	auto ent = parseExpandEntity(it, ents);
	if (!ent.getContent().isTuple())
		throw runtime_error("expand entity within tuple must be a tuple");
	return fillTemplateTuple(params, templateTuple, ent.getContent().getTuple(), index);
}

Tuple::size_type Parser::fillTemplateTuple(const TemplateHeadStandard::Parameters& params, Tuple& templateTuple, const Tuple& expandTuple, Tuple::size_type index = 0)
{
	for (const auto& item : expandTuple.getOrderedKeyValues())
	{
		if (item.first == nullptr)
			templateTuple.at(index) = checkTemplateContainer(params, params.at(index), *item.second);
		else
			templateTuple.at(*item.first) = checkTemplateContainer(params, params.at(*item.first), *item.second);
		++index;
	}
	return index;
}

Webss Parser::fillTemplateBodyStandard(const TemplateHeadStandard::Parameters& params, const Webss& templateItem)
{
	switch (templateItem.getType())
	{
	case WebssType::DICTIONARY:
		//...
	case WebssType::LIST: case WebssType::LIST_TEXT:
		//...
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
	{
		//build tuple with the param's template head and the expanded tuple's item
		Tuple templateTuple(params.getSharedKeys());
		if (fillTemplateTuple(params, templateTuple, templateItem.getTuple()) < templateTuple.size())
			checkDefaultValues(templateTuple, params);
		return templateTuple;
	}
	default:
		throw runtime_error("template head must be implemented");
	}
}

Webss Parser::checkTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& param, const Webss& tupleItem)
{
	switch (param.getTypeThead())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		throw runtime_error("can't expand for a binary template");
	case WebssType::TEMPLATE_HEAD_SELF:
		return fillTemplateBodyStandard(params, tupleItem);
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		return fillTemplateBodyStandard(param.getTemplateHeadStandard().getParameters(), tupleItem);
	default:
		return tupleItem;
	}
}