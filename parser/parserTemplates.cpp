//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "paramDocumentIncluder.hpp"
#include "patternsContainers.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";

void setDefaultValue(Webss& value, const ParamStandard& defaultValue);

template <class Parameters>
Tuple makeDefaultTuple(const Parameters& params)
{
	Tuple tuple(params.getSharedKeys());
	for (Tuple::size_type i = 0; i < params.size(); ++i)
		setDefaultValue(tuple[i], params[i]);
	return tuple;
}

void setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasTemplateHead())
		value = makeDefaultTuple(defaultValue.getTemplateHeadStandard().getParameters());
	else if (!defaultValue.hasDefaultValue())
		throw runtime_error(ERROR_NO_DEFAULT);
	else
		value = Webss(defaultValue.getDefaultPointer());
}

template <class Parameters>
void checkDefaultValues(Tuple& tuple, const Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}

template <>
void checkDefaultValues<TemplateHeadBinary::Parameters>(Tuple&, const TemplateHeadBinary::Parameters&) {} //already checked while parsing binary

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
		case Tag::NAME_START:
			return checkTemplateBodyEntity(params, WebssType::DICTIONARY);
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
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		ContainerSwitcher switcher(*this, ConType::TUPLE, true);
		if (!containerEmpty())
		{
			do
			{
				switch (nextTag)
				{
				case Tag::EXPAND:
				{
					auto ent = parseExpandEntity();
					switch (ent.getContent().getType())
					{
					case WebssType::DICTIONARY:
						for (const auto& item : ent.getContent().getDictionary())
						{
							tuple.at(item.first) = checkTemplateContainer(params, params.at(item.first), item.second);
							++index;
						}
						break;
					case WebssType::LIST: case WebssType::LIST_TEXT:
						for (const auto& item : ent.getContent().getList())
						{
							tuple.at(index) = checkTemplateContainer(params, params.at(index), item);
							++index;
						}
						break;
					case WebssType::TUPLE: case WebssType::TUPLE_TEXT: case WebssType::TUPLE_ABSTRACT:
						for (const auto& item : ent.getContent().getTuple())
						{
							tuple.at(index) = checkTemplateContainer(params, params.at(index), item);
							++index;
						}
						break;
					default:
						throw runtime_error("expand entity in tuple must be a dictionary, list, or tuple");
					}
					continue;
				}
				case Tag::SEPARATOR: //void
					break;
				case Tag::EXPLICIT_NAME:
				{
					auto name = parseNameExplicit();
					nextTag = getTag(it);
					tuple.at(name) = isText ? Webss(parseLineString()) : parseTemplateContainer(params, params.at(name));
					break;
				}
				case Tag::NAME_START:
					if (isText)
						tuple.at(index) = Webss(parseLineString());
					else
					{
						auto nameType = parseNameType();
						if (nameType.type == NameType::NAME)
						{
							nextTag = getTag(it);
							tuple.at(nameType.name) = parseTemplateContainer(params, params.at(nameType.name));
						}
						else
						{
							if (params.at(index).hasTemplateHead())
								throw runtime_error(ERROR_UNEXPECTED);
							switch (nameType.type)
							{
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
					}
					break;
				default:
					tuple.at(index) = isText ? Webss(parseLineString()) : parseTemplateContainer(params, params.at(index));
					break;
				}
				++index;
			} while (checkNextElement());
		}
		checkDefaultValues(tuple, params);
		return tuple;
	}

private:
	template <class Parameters>
	Webss checkTemplateBodyEntity(const Parameters& params, WebssType filter)
	{
		auto value = parseValueOnly();
		switch (value.getType())
		{
		case WebssType::DICTIONARY:
			if (filter != WebssType::DICTIONARY)
				throw runtime_error(ERROR_UNEXPECTED);
			return checkTemplateBodyEntityDict(params, value.getDictionary());
		case WebssType::LIST:
			if (filter == WebssType::TUPLE)
				throw runtime_error(ERROR_UNEXPECTED);
			return checkTemplateBodyEntityList(params, value.getList());
		case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
			return checkTemplateBodyEntityTuple(params, value.getTuple());
		default:
			throw runtime_error("expected entity with template body structure");
		}
	}

	template <class Parameters>
	Dictionary checkTemplateBodyEntityDict(const Parameters& params, const Dictionary& entityDict)
	{
		Dictionary dict;
		for (const auto& keyValue : entityDict)
		{
			if (keyValue.second.isList())
				dict.add(keyValue.first, checkTemplateBodyEntityList(params, keyValue.second.getList()));
			else
				dict.add(keyValue.first, checkTemplateBodyEntityTuple(params, keyValue.second.getTuple()));
		}
		return dict;
	}

	template <class Parameters>
	List checkTemplateBodyEntityList(const Parameters& params, const List& entityList)
	{
		List list;
		for (const auto& elem : entityList)
			list.add(checkTemplateBodyEntityTuple(params, elem.getTuple()));
		return list;
	}

	template <class Parameters>
	Tuple checkTemplateBodyEntityTuple(const Parameters& params, const Tuple& entityTuple)
	{
		Tuple tuple(params.getSharedKeys(), entityTuple.getData());
		checkDefaultValues(tuple, params);
		return tuple;
	}

	template <class Parameters>
	Dictionary parseTemplateDictionary(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), [&](Dictionary& dict)
		{
			if (nextTag == Tag::EXPAND)
				expandDictionary(dict);
			else
			{
				string name = parseNameDictionary();
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
				case Tag::NAME_START:
					dict.addSafe(move(name), checkTemplateBodyEntity(params, WebssType::LIST));
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
		return parseContainer<List, ConType::LIST>(List(), [&](List& list)
		{
			switch (nextTag)
			{
			case Tag::EXPAND:
				expandList(list);
				break;
			case Tag::START_TUPLE:
				list.add(funcTemplTupleRegular(params));
				break;
			case Tag::TEXT_TUPLE:
				list.add(funcTemplTupleText(params));
				break;
			case Tag::NAME_START:
				list.add(checkTemplateBodyEntity(params, WebssType::TUPLE));
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
	case WebssType::TEMPLATE_HEAD_SCOPED:
	{
		auto head = move(headWebss.getTemplateHeadScopedRaw());
		auto body = parseTemplateBodyScoped(head.getParameters());
		return TemplateScoped(move(head), move(body));
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

Webss Parser::parseTemplateBodyScoped(const TemplateHeadScoped::Parameters& params)
{
	ParamDocumentIncluder includer(ents, params);
	nextTag = getTag(it);
	return parseValueOnly();
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
	case WebssType::TEMPLATE_HEAD_SCOPED:
		return parseTemplateBodyScoped(defaultValue.getTemplateHeadScoped().getParameters());
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



Webss Parser::checkTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& param, const Webss& value)
{
	switch (param.getTypeThead())
	{
	case WebssType::TEMPLATE_HEAD_BINARY:
		throw runtime_error("can't expand for a binary template");
	case WebssType::TEMPLATE_HEAD_SCOPED:
		throw runtime_error("can't expand for a scoped template");
	case WebssType::TEMPLATE_HEAD_SELF:
		throw runtime_error("can't expand for a self template");
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		switch (value.getType())
		{
		case WebssType::DICTIONARY:
			//...
		case WebssType::LIST:
			//...
		case WebssType::TUPLE:
		{
			const auto& params2 = param.getTemplateHeadStandard().getParameters();
			Tuple tuple(params2.getSharedKeys());
			Tuple::size_type index = 0;
			const auto& valueTuple = value.getTuple();
			if (valueTuple.size() > tuple.size())
				throw runtime_error("tuple to implement is too big");
			for (Tuple::size_type i = 0; i < valueTuple.size(); ++i)
				tuple[index] = checkTemplateContainer(params2, params2[i], valueTuple[i]);
			return tuple;
		}
		default:
			throw runtime_error("template head must be implemented");
		}
	default:
		return value;
	}
}