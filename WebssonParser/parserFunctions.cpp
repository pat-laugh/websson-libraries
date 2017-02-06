//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"
#include "utilsParser.h"

using namespace std;
using namespace webss;

template <class Parameters>
void checkDefaultValues(Tuple& tuple, const Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).type == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}

class ParserTemplates : public Parser
{
public:
	Webss parseTemplateBodyScoped(It& it, const TemplateHeadScoped::Parameters& params, ConType con)
	{
		ParamDocumentIncluder includer(ents, params);
		return parseValueOnly(it, con);
	}

	Webss parseTemplateBodyStandard(It& it, const TemplateHeadStandard::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadStandard::Parameters>(it, params, [&](It& it, const TemplateHeadStandard::Parameters& params) { return parseTemplateTupleStandard(it, params); }, [&](It& it, const TemplateHeadStandard::Parameters& params) { return parseTemplateTupleText(it, params); });
	}

	Webss parseTemplateBodyText(It& it, const TemplateHeadText::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadText::Parameters>(it, params, [&](It& it, const TemplateHeadText::Parameters& params) { return parseTemplateTupleText(it, params); }, [&](It& it, const TemplateHeadText::Parameters& params) { return parseTemplateTupleText(it, params); });
	}

	template <class Parameters>
	Webss parseTemplateBody(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& templTupleRegular, function<Tuple(It& it, const Parameters& params)>&& templTupleText)
	{
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::DICTIONARY:
			return parseTemplateDictionary<Parameters>(++it, params, move(templTupleRegular), move(templTupleText));
		case TypeContainer::LIST:
			return parseTemplateListRegular<Parameters>(++it, params, move(templTupleRegular), move(templTupleText));
		case TypeContainer::TUPLE:
			return templTupleRegular(++it, params);
		case TypeContainer::TEXT_LIST:
			return parseTemplateListText<Parameters>(++it, params, move(templTupleText));
		case TypeContainer::TEXT_TUPLE:
			return templTupleText(++it, params);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

private:
	template <class Parameters>
	Dictionary parseTemplateDictionary(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& templTupleRegular, function<Tuple(It& it, const Parameters& params)>&& templTupleText)
	{
		return parseContainer<Dictionary, ConType::DICTIONARY>(it, Dictionary(), [&](Dictionary& dict, ConType con)
		{
			if (!isNameStart(*it))
				throw runtime_error(ERROR_UNEXPECTED);
			auto name = parseNameSafe(it);
			switch (skipJunkToContainer(it))
			{
			case TypeContainer::LIST:
				dict.addSafe(move(name), parseTemplateListRegular<Parameters>(++it, params, move(templTupleRegular), move(templTupleText)));
				break;
			case TypeContainer::TUPLE:
				dict.addSafe(move(name), templTupleRegular(++it, params));
				break;
			case TypeContainer::TEXT_LIST:
				dict.addSafe(move(name), parseTemplateListText<Parameters>(++it, params, move(templTupleText)));
				break;
			case TypeContainer::TEXT_TUPLE:
				dict.addSafe(move(name), templTupleText(++it, params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		});
	}

	template <class Parameters>
	List parseTemplateListRegular(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& templTupleRegular, function<Tuple(It& it, const Parameters& params)>&& templTupleText)
	{
		return parseContainer<List, ConType::LIST>(it, List(), [&](List& list, ConType con)
		{
			auto nextCont = skipJunkToContainer(it);
			if (nextCont == TypeContainer::TUPLE)
				list.add(templTupleRegular(++it, params));
			else if (nextCont == TypeContainer::TEXT_TUPLE)
				list.add(templTupleText(++it, params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	template <class Parameters>
	List parseTemplateListText(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& templTupleText)
	{
		return parseContainer<List, ConType::LIST>(it, List(true), [&](List& list, ConType con)
		{
			auto nextCont = skipJunkToContainer(it);
			if (nextCont == TypeContainer::TUPLE || nextCont == TypeContainer::TEXT_TUPLE)
				list.add(templTupleText(++it, params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	Webss parseTemplateContainer(It& it, const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue)
	{
		static const ConType CON = ConType::TUPLE;
		switch (defaultValue.getTypeFhead())
		{
		case WebssType::TEMPLATE_HEAD_BINARY:
			return parseTemplateBodyBinary(it, defaultValue.getTemplateHeadBinary().getParameters());
		case WebssType::TEMPLATE_HEAD_SCOPED:
			return parseTemplateBodyScoped(it, defaultValue.getTemplateHeadScoped().getParameters(), CON);
		case WebssType::TEMPLATE_HEAD_SELF:
			return parseTemplateBodyStandard(it, params);
		case WebssType::TEMPLATE_HEAD_STANDARD:
			return parseTemplateBodyStandard(it, defaultValue.getTemplateHeadStandard().getParameters());
		case WebssType::TEMPLATE_HEAD_TEXT:
			return parseTemplateBodyText(it, defaultValue.getTemplateHeadText().getParameters());
		default:
			return parseValueOnly(it, CON);
		}
	}

	Tuple parseTemplateTupleStandard(It& it, const TemplateHeadStandard::Parameters& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		if (!checkEmptyContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }))
		{
			do
			{
				if (!isNameStart(*it))
					tuple.at(index) = parseTemplateContainer(it, params, params.at(index));
				else
				{
					auto nameType = parseNameType(it);
					if (nameType.type == NameType::NAME)
						tuple.at(nameType.name) = parseTemplateContainer(it, params, params.at(nameType.name));
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
							auto otherValue = checkAbstractEntity(it, CON, nameType.entity);
							if (otherValue.type != OtherValue::VALUE_ONLY)
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(otherValue.value);
							break;
						}
						case NameType::ENTITY_CONCRETE:
							tuple.at(index) = move(nameType.entity);
							break;
						default:
							assert(false); throw domain_error("");
						}
					}
					
				}
				++index;
			} while (checkNextElementContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }));
		}
		checkDefaultValues(tuple, params);
		return tuple;
	}

	template <class Parameters>
	Tuple parseTemplateTupleText(It& it, const Parameters& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys(), true);
		Tuple::size_type index = 0;
		if (!checkEmptyContainerVoid(it, CON, [&]() { ++index; }))
			do
				tuple.at(index++) = parseLineString(it, CON);
		while (checkNextElementContainerVoid(it, CON, [&]() { ++index; }));
		checkDefaultValues(tuple, params);
		return tuple;
	}
};

Webss Parser::parseTemplate(It& it, ConType con)
{
	auto headWebss = parseTemplateHead(it);
	switch (headWebss.type)
	{
	case WebssType::BLOCK_HEAD:
		return Block(move(*headWebss.blockHead), parseValueOnly(it, con));
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		auto head = move(*headWebss.fheadBinary);
		auto body = parseTemplateBodyBinary(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SCOPED:
	{
		auto head = move(*headWebss.fheadScoped);
		auto body = parseTemplateBodyScoped(it, head.getParameters(), con);
		return TemplateScoped(move(head), move(body));
	}
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		auto head = move(*headWebss.fheadStandard);
		auto body = parseTemplateBodyStandard(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SELF:
		throw runtime_error("self in a thead must be within a non-empty thead");
	default:
		throw logic_error("");
	}
}

Webss Parser::parseTemplateText(It& it)
{
	auto head = parseTemplateHeadText(it);
	auto body = parseTemplateBodyText(it, head.getParameters());
	return{ move(head), move(body) };
}

Webss Parser::parseTemplateBodyBinary(It& it, const TemplateHeadBinary::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBody<TemplateHeadBinary::Parameters>(it, params, [&](It& it, const TemplateHeadBinary::Parameters& params) { return parseTemplateTupleBinary(it, params); }, [&](It& it, const TemplateHeadBinary::Parameters& params) -> Tuple { throw runtime_error(ERROR_UNEXPECTED); });
}

Webss Parser::parseTemplateBodyScoped(It& it, const TemplateHeadScoped::Parameters& params, ConType con)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyScoped(it, params, con);
}

Webss Parser::parseTemplateBodyStandard(It& it, const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyStandard(it, params);
}

Webss Parser::parseTemplateBodyText(It& it, const TemplateHeadText::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyText(it, params);
}