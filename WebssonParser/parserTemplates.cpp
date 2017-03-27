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
	Webss parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadStandard::Parameters>(params, [&](const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleStandard(params)); }, [&](const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(params), true); });
	}

	Webss parseTemplateBodyText(const TemplateHeadStandard::Parameters& params)
	{
		return parseTemplateBody<TemplateHeadStandard::Parameters>(params, [&](const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(params), true); }, [&](const TemplateHeadStandard::Parameters& params) { return Webss(parseTemplateTupleText(params), true); });
	}

	template <class Parameters>
	Webss parseTemplateBody(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		switch (getTag(it))
		{
		case Tag::START_DICTIONARY:
			++it;
			return parseTemplateDictionary<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_LIST:
			++it;
			return parseTemplateList<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_TUPLE:
			++it;
			return funcTemplTupleRegular(params);
		case Tag::TEXT_TUPLE:
			++it;
			return funcTemplTupleText(params);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

private:
	template <class Parameters>
	Dictionary parseTemplateDictionary(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), [&](Dictionary& dict, Parser& parser)
		{
			if (!isNameStart(*it))
				throw runtime_error(ERROR_UNEXPECTED);
			auto name = parser.parseNameSafe();
			switch (getTag(it))
			{
			case Tag::START_LIST:
				++it;
				dict.addSafe(move(name), parseTemplateList<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText)));
				break;
			case Tag::START_TUPLE:
				++it;
				dict.addSafe(move(name), funcTemplTupleRegular(params));
				break;
			case Tag::TEXT_TUPLE:
				++it;
				dict.addSafe(move(name), funcTemplTupleText(params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		});
	}

	template <class Parameters>
	List parseTemplateList(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<List, ConType::LIST>(List(), [&](List& list, Parser& parser)
		{
			auto tag = getTag(it);
			if (tag == Tag::START_TUPLE)
			{
				++it;
				list.add(funcTemplTupleRegular(params));
			}
			else if (tag == Tag::TEXT_TUPLE)
			{
				++it;
				list.add(funcTemplTupleText(params));
			}
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	Webss parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue)
	{
		static const ConType CON = ConType::TUPLE;
		switch (defaultValue.getTypeThead())
		{
		case WebssType::TEMPLATE_HEAD_BINARY:
			return parseTemplateBodyBinary(defaultValue.getTemplateHeadBinary().getParameters());
		case WebssType::TEMPLATE_HEAD_SCOPED:
			return parseTemplateBodyScoped(defaultValue.getTemplateHeadScoped().getParameters(), CON);
		case WebssType::TEMPLATE_HEAD_SELF:
			return parseTemplateBodyStandard(params);
		case WebssType::TEMPLATE_HEAD_STANDARD:
			return parseTemplateBodyStandard(defaultValue.getTemplateHeadStandard().getParameters());
		case WebssType::TEMPLATE_HEAD_TEXT:
			return parseTemplateBodyText(defaultValue.getTemplateHeadStandard().getParameters());
		default:
			return parseValueOnly(CON);
		}
	}

	Tuple parseTemplateTupleStandard(const TemplateHeadStandard::Parameters& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		Parser parser(*this, CON, true);
		if (!parser.parserContainerEmpty())
		{
			do
			{
				if (!isNameStart(*it))
					tuple.at(index) = parseTemplateContainer(params, params.at(index)); //needs parser.
				else
				{
					auto nameType = parseNameType();
					if (nameType.type == NameType::NAME)
						tuple.at(nameType.name) = parseTemplateContainer(params, params.at(nameType.name)); //needs parser.
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
							auto otherValue = parser.checkAbstractEntity(CON, nameType.entity);
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
			} while (parserCheckNextElement());
		}
		checkDefaultValues(tuple, params);
		return tuple;
	}

	template <class Parameters>
	Tuple parseTemplateTupleText(const Parameters& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		Parser parser(*this, CON, true);
		if (!parser.parserContainerEmpty())
			do
				tuple.at(index++) = parser.parseLineString(CON);
			while (parserCheckNextElement());
		checkDefaultValues(tuple, params);
		return tuple;
	}
};

Webss GlobalParser::Parser::parseTemplate(ConType con)
{
	auto headWebss = parseTemplateHead();
	switch (headWebss.getType())
	{
	case WebssType::BLOCK_HEAD:
		return Block(move(headWebss.getBlockHead()), parseValueOnly(con));
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		auto head = move(headWebss.getTemplateHeadBinary());
		auto body = parseTemplateBodyBinary(head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SCOPED:
	{
		auto head = move(headWebss.getTemplateHeadScoped());
		auto body = parseTemplateBodyScoped(head.getParameters(), con);
		return TemplateScoped(move(head), move(body));
	}
	case WebssType::TEMPLATE_HEAD_SELF:
		throw runtime_error("self in a thead must be within a non-empty thead");
	case WebssType::TEMPLATE_HEAD_STANDARD:
	{
		auto head = move(headWebss.getTemplateHeadStandard());
		auto body = parseTemplateBodyStandard(head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_TEXT:
	{
		auto head = move(headWebss.getTemplateHeadStandard());
		auto body = parseTemplateBodyText(head.getParameters());
		return{ move(head), move(body), true };
	}
	default:
		throw logic_error("");
	}
}

Webss GlobalParser::Parser::parseTemplateText()
{
	auto head = parseTemplateHeadText();
	auto body = parseTemplateBodyText(head.getParameters());
	return{ move(head), move(body), true };
}

Webss GlobalParser::Parser::parseTemplateBodyBinary(const TemplateHeadBinary::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBody<TemplateHeadBinary::Parameters>(params, [&](const TemplateHeadBinary::Parameters& params) { return parseTemplateTupleBinary(params); }, [&](const TemplateHeadBinary::Parameters& params) -> Webss { throw runtime_error(ERROR_UNEXPECTED); });
}

Webss GlobalParser::Parser::parseTemplateBodyScoped(const TemplateHeadScoped::Parameters& params, ConType con)
{
	ParamDocumentIncluder includer(ents, params);
	return parseValueOnly(con);
}

Webss GlobalParser::Parser::parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyStandard(params);
}

Webss GlobalParser::Parser::parseTemplateBodyText(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyText(params);
}