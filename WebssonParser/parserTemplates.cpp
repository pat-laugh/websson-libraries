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
		switch (nextTag = getTag(it))
		{
		case Tag::START_DICTIONARY:
			return parseTemplateDictionary<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_LIST:
			return parseTemplateList<Parameters>(params, move(funcTemplTupleRegular), move(funcTemplTupleText));
		case Tag::START_TUPLE:
			return funcTemplTupleRegular(params);
		case Tag::TEXT_TUPLE:
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
		});
	}

	template <class Parameters>
	List parseTemplateList(const Parameters& params, function<Webss(const Parameters& params)>&& funcTemplTupleRegular, function<Webss(const Parameters& params)>&& funcTemplTupleText)
	{
		return parseContainer<List, ConType::LIST>(List(), [&](List& list, Parser& parser)
		{
			if (nextTag == Tag::START_TUPLE)
				list.add(funcTemplTupleRegular(params));
			else if (nextTag == Tag::TEXT_TUPLE)
				list.add(funcTemplTupleText(params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	Tuple parseTemplateTupleStandard(const TemplateHeadStandard::Parameters& params)
	{
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		Parser parser(*this, ConType::TUPLE, true);
		if (!parser.parserContainerEmpty())
		{
			do
			{
				if (!isNameStart(*it))
					tuple.at(index) = parser.parseTemplateContainer(params, params.at(index));
				else
				{
					auto nameType = parseNameType();
					if (nameType.type == NameType::NAME)
						tuple.at(nameType.name) = parser.parseTemplateContainer(params, params.at(nameType.name));
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
							auto otherValue = parser.checkAbstractEntity(nameType.entity);
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
			} while (parser.parserCheckNextElement());
		}
		checkDefaultValues(tuple, params);
		return tuple;
	}

	template <class Parameters>
	Tuple parseTemplateTupleText(const Parameters& params)
	{
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		Parser parser(*this, ConType::TUPLE, true);
		if (!parser.parserContainerEmpty())
			do
				tuple.at(index++) = parser.parseLineString();
			while (parser.parserCheckNextElement());
		checkDefaultValues(tuple, params);
		return tuple;
	}
};

Webss GlobalParser::Parser::parseTemplate()
{
	auto headWebss = parseTemplateHead();
	switch (headWebss.getType())
	{
	case WebssType::BLOCK_HEAD:
		nextTag = getTag(it);
		return Block(move(headWebss.getBlockHead()), parseValueOnly());
	case WebssType::TEMPLATE_HEAD_BINARY:
	{
		auto head = move(headWebss.getTemplateHeadBinary());
		auto body = parseTemplateBodyBinary(head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::TEMPLATE_HEAD_SCOPED:
	{
		auto head = move(headWebss.getTemplateHeadScoped());
		auto body = parseTemplateBodyScoped(head.getParameters());
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

Webss GlobalParser::Parser::parseTemplateBodyScoped(const TemplateHeadScoped::Parameters& params)
{
	ParamDocumentIncluder includer(ents, params);
	nextTag = getTag(it);
	return parseValueOnly();
}

Webss GlobalParser::Parser::parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyStandard(params);
}

Webss GlobalParser::Parser::parseTemplateBodyText(const TemplateHeadStandard::Parameters& params)
{
	return static_cast<ParserTemplates*>(this)->parseTemplateBodyText(params);
}

Webss GlobalParser::Parser::parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue)
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
		nextTag = getTag(it);
		return parseValueOnly();
	}
}