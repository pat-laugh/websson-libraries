//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

Parser::NameType Parser::parseNameType()
{
	string name = parseName(it);
	if (isKeyword(name))
		return{ Keyword(name) };
	else if (!ents.hasEntity(name))
		return{ move(name) };

	const Entity* ent = &ents[name];
scopeLoop:
	if (getTag(it) != Tag::SCOPE)
		return{ *ent };
	try
	{
		skipJunkToTag(++it, Tag::NAME_START);
		const auto& content = ent->getContent();
		if (content.isEnum())
			return content.getEnumSafe().at(parseName(it));
		ent = &content.getNamespaceSafe().at(parseName(it));
		goto scopeLoop;
	}
	catch (const exception&) { throw runtime_error("could not get scoped value"); }
}

#define CASE_TAG_KEY_CHAR Tag::START_DICTIONARY: case Tag::START_LIST: case Tag::START_TUPLE: case Tag::START_TEMPLATE: case Tag::LINE_STRING: case Tag::EQUAL: case Tag::C_STRING: case Tag::TEXT_DICTIONARY: case Tag::TEXT_LIST: case Tag::TEXT_TUPLE: case Tag::TEXT_TEMPLATE

Webss Parser::parseCharValue()
{
	switch (nextTag)
	{
	case Tag::START_DICTIONARY: return parseDictionary();
	case Tag::START_LIST: return parseList();
	case Tag::START_TUPLE: return parseTuple();
	case Tag::START_TEMPLATE: return parseTemplate();
	case Tag::LINE_STRING: ++it; return parseLineString();
	case Tag::EQUAL: ++it; return parseValueEqual();
	case Tag::C_STRING: return parseCString();
	case Tag::TEXT_DICTIONARY: return parseMultilineString();
	case Tag::TEXT_LIST: return{ parseListText(), true };
	case Tag::TEXT_TUPLE: return{ parseTupleText(), true };
	case Tag::TEXT_TEMPLATE: return parseTemplateText();
	default: assert(false);
	}
}

void Parser::addJsonKeyvalue(Dictionary& dict)
{
	try
	{
		skipJunkToTag(++it, Tag::NAME_START);
		auto name = parseNameSafe();
		skipJunkToTag(it, Tag::C_STRING);
		if (*skipJunkToValid(++it) != CHAR_COLON)
			throw runtime_error("");
		++it;
		dict.addSafe(move(name), parseValueEqual());
	}
	catch (const runtime_error&)
	{
		throw runtime_error("could not parse supposed Json key-value");
	}
}

Webss Parser::parseValueEqual()
{
	if (*skipJunkToValid(it) != CHAR_EQUAL)
	{
		nextTag = getTag(it);
		return parseValueOnly();
	}
	throw runtime_error("expected value-only not starting with an equal sign");
}

Parser::OtherValue Parser::parseOtherValue()
{
	switch (nextTag)
	{
	case CASE_TAG_KEY_CHAR:
		return parseCharValue();
	case Tag::NAME_START:
	{
		auto nameType = parseNameType();
		switch (nameType.type)
		{
		case NameType::NAME:
			switch (nextTag = getTag(it))
			{
			case CASE_TAG_KEY_CHAR:
				return OtherValue(move(nameType.name), parseCharValue());
			default:
				return{ move(nameType.name) };
			}
		case NameType::KEYWORD:
			return{ nameType.keyword };
		case NameType::ENTITY_ABSTRACT:
			return checkAbstractEntity(nameType.entity);
		case NameType::ENTITY_CONCRETE:
			return{ Webss(move(nameType.entity)) };
		default:
			assert(false); throw domain_error("");
		}
	}
	case Tag::NUMBER_START:
		return parseNumber();
	case Tag::EXPLICIT_NAME:
	{
		auto name = parseExplicitName();
		switch (nextTag = getTag(it))
		{
		case CASE_TAG_KEY_CHAR:
			return OtherValue(move(name), parseCharValue());
		default:
			return{ name };
		}
	}
	default:
		throw runtime_error(nextTag == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}
}

bool isTemplateBodyStart(Tag tag)
{
	return tag == Tag::START_TUPLE || tag == Tag::START_LIST || tag == Tag::START_DICTIONARY;
}

Parser::OtherValue Parser::checkAbstractEntity(const Entity& ent)
{
	const auto& content = ent.getContent();
	nextTag = getTag(it);
	switch (content.getTypeSafe())
	{
	case WebssType::BLOCK_HEAD:
		return{ Block(ent, parseValueOnly()) };
	case WebssType::TEMPLATE_HEAD_BINARY:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadBinary(ent), parseTemplateBodyBinary(content.getTemplateHeadBinarySafe().getParameters())) };
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		switch (nextTag)
		{
		case CASE_TAG_KEY_CHAR: case Tag::NAME_START: case Tag::NUMBER_START:
			return{ TemplateScoped(ent, parseTemplateBodyScoped(ent.getContent().getTemplateHeadScopedSafe().getParameters())) };
		default:
			break;
		}
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyStandard(content.getTemplateHeadStandardSafe().getParameters())) };
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyText(content.getTemplateHeadStandardSafe().getParameters())) };
		break;
	default:
		break;
	}
	return{ ent };
}

void Parser::parseOtherValue(std::function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Entity& abstractEntity)> funcAbstractEntity)
{
	auto other = parseOtherValue();
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		funcKeyValue(move(other.key), move(other.value));
		break;
	case OtherValue::Type::KEY_ONLY:
		funcKeyOnly(move(other.key));
		break;
	case OtherValue::Type::VALUE_ONLY:
		funcValueOnly(move(other.value));
		break;
	case OtherValue::Type::ABSTRACT_ENTITY:
		funcAbstractEntity(other.abstractEntity);
		break;
	default:
		assert(false); throw domain_error("");
	}
}

Webss Parser::parseValueOnly()
{
	auto otherValue = parseOtherValue();
	if (otherValue.type != OtherValue::VALUE_ONLY)
		throw runtime_error("expected value-only");
	return move(otherValue.value);
}