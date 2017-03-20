//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "utilsParser.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

template <class Parameters>
void checkDefaultValues(Tuple& tuple, const Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getType() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}

class ParserTemplates : public GlobalParser::Parser
{
public:
	Webss parseTemplateBodyStandard(It& it, const TemplateHeadStandard::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadStandard::Parameters>(it, params, [&](It& it, const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleStandard(it, params)); }, [&](It& it, const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(it, params), true); });
	}

	Webss parseTemplateBodyText(It& it, const TemplateHeadStandard::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadStandard::Parameters>(it, params, [&](It& it, const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(it, params), true); }, [&](It& it, const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(it, params), true); });
	}

	template <class Parameters>
	Webss parseTemplateBody(It& it, const Parameters& params, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleRegular, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleText)
	{
		switch (getTag(it))
		{
		case Tag::START_DICTIONARY:
			return parseTemplateDictionary<Parameters>(++it, params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_LIST:
			return parseTemplateList<Parameters>(++it, params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_TUPLE:
			return funcTemplTupleRegular(++it, params);
		case Tag::TEXT_TUPLE:
			return funcTemplTupleText(++it, params);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

private:
	template <class Parameters>
	Dictionary parseTemplateDictionary(It& it, const Parameters& params, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleRegular, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<Dictionary, ConType::DICTIONARY>(it, Dictionary(), [&](Dictionary& dict, ConType con)
		{
			if (!isNameStart(*it))
				throw runtime_error(ERROR_UNEXPECTED);
			auto name = parseNameSafe(it);
			switch (getTag(it))
			{
			case Tag::START_LIST:
				dict.addSafe(move(name), parseTemplateList<Parameters>(++it, params, move(funcTemplTupleRegular), move(funcTemplTupleText)));
				break;
			case Tag::START_TUPLE:
				dict.addSafe(move(name), funcTemplTupleRegular(++it, params));
				break;
			case Tag::TEXT_TUPLE:
				dict.addSafe(move(name), funcTemplTupleText(++it, params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		});
	}

	template <class Parameters>
	List parseTemplateList(It& it, const Parameters& params, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleRegular, function<Webss(It& it, const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<List, ConType::LIST>(it, List(), [&](List& list, ConType con)
		{
			auto tag = getTag(it);
			if (tag == Tag::START_TUPLE)
				list.add(funcTemplTupleRegular(++it, params));
			else if (tag == Tag::TEXT_TUPLE)
				list.add(funcTemplTupleText(++it, params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	Webss parseTemplateContainer(It& it, const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue)
	{
		static const ConType CON = ConType::TUPLE;
		switch (defaultValue.getTypeThead())
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
			return parseTemplateBodyText(it, defaultValue.getTemplateHeadStandard().getParameters());
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
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		if (!checkEmptyContainerVoid(it, CON, [&]() { ++index; }))
			do
				tuple.at(index++) = parseLineString(it, CON);
		while (checkNextElementContainerVoid(it, CON, [&]() { ++index; }));
		checkDefaultValues(tuple, params);
		return tuple;
	}
};

Webss GlobalParser::Parser::parseTemplate(It& it, ConType con)
{
	auto headWebss = parseTemplateHead(it);
	switch (headWebss.getType())
	{
	case WebssType::BLOCK_HEAD:
		return Block(move(headWebss.getBlockHead()), parseValueOnly(it, con));
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		auto head = move(headWebss.getTemplateHeadBinary());
		auto body = parseTemplateBodyBinary(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SCOPED:
	{
		auto head = move(headWebss.getTemplateHeadScoped());
		auto body = parseTemplateBodyScoped(it, head.getParameters(), con);
		return TemplateScoped(move(head), move(body));
	}
	case WebssType::TEMPLATE_HEAD_SELF:
		throw runtime_error("self in a thead must be within a non-empty thead");
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		auto head = move(headWebss.getTemplateHeadStandard());
		auto body = parseTemplateBodyStandard(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_TEXT:
	{
		auto head = move(headWebss.getTemplateHeadStandard());
		auto body = parseTemplateBodyText(it, head.getParameters());
		return{ move(head), move(body), true };
	}
	default:
		throw logic_error("");
	}
}

Webss GlobalParser::Parser::parseTemplateText(It& it)
{
	auto head = parseTemplateHeadText(it);
	auto body = parseTemplateBodyText(it, head.getParameters());
	return{ move(head), move(body), true };
}

Webss GlobalParser::Parser::parseTemplateBodyBinary(It& it, const TemplateHeadBinary::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBody<TemplateHeadBinary::Parameters>(it, params, [&](It& it, const TemplateHeadBinary::Parameters& params) { return parseTemplateTupleBinary(it, params); }, [&](It& it, const TemplateHeadBinary::Parameters& params) -> Webss { throw runtime_error(ERROR_UNEXPECTED); });
}

Webss GlobalParser::Parser::parseTemplateBodyScoped(It& it, const TemplateHeadScoped::Parameters& params, ConType con)
{
	ParamDocumentIncluder includer(ents, params);
	return parseValueOnly(it, con);
}

Webss GlobalParser::Parser::parseTemplateBodyStandard(It& it, const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyStandard(it, params);
}

Webss GlobalParser::Parser::parseTemplateBodyText(It& it, const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyText(it, params);
}