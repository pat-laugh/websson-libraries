//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "errors.hpp"
#include "nameType.hpp"
#include "parserNumbers.hpp"
#include "parserStrings.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Webss Parser::parseValueEqual()
{
	if ((nextTag = getTag(it)) != Tag::EQUAL)
		return parseValueOnly();
	throw runtime_error("expected value-only not starting with an equal sign");
}

#define CASE_TAG_KEY_CHAR Tag::START_DICTIONARY: case Tag::START_LIST: case Tag::START_TUPLE: case Tag::START_TEMPLATE: case Tag::LINE_STRING: case Tag::EQUAL: case Tag::C_STRING: case Tag::TEXT_DICTIONARY: case Tag::TEXT_LIST: case Tag::TEXT_TUPLE: case Tag::TEXT_TEMPLATE

Parser::OtherValue Parser::parseOtherValue(bool explicitName)
{
	switch (nextTag)
	{
	case Tag::START_DICTIONARY: return Webss(parseDictionary());
	case Tag::START_LIST: return Webss(parseList());
	case Tag::START_TUPLE: return Webss(parseTuple());
	case Tag::START_TEMPLATE: return Webss(parseTemplate());
	case Tag::LINE_STRING: ++it; return Webss(parseLineString(*this));
	case Tag::EQUAL: ++it; return Webss(parseValueEqual());
	case Tag::C_STRING: return Webss(parseCString(*this));
	case Tag::TEXT_DICTIONARY: return Webss(parseMultilineString(*this));
	case Tag::TEXT_LIST: return Webss(parseListText(), WebssType::LIST_TEXT);
	case Tag::TEXT_TUPLE: return Webss(parseTupleText(), WebssType::TUPLE_TEXT);
	case Tag::TEXT_TEMPLATE: return Webss(parseTemplateText());
	case Tag::NAME_START:
		if (explicitName)
			return parseOtherValueName(parseName(it));
		else
		{
			auto nameType = parseNameType(it, ents);
			switch (nameType.type)
			{
			case NameType::NAME:
				return parseOtherValueName(move(nameType.name));
			case NameType::KEYWORD:
				return Webss(nameType.keyword);
			case NameType::ENTITY_ABSTRACT:
				return checkAbstractEntity(nameType.entity);
			case NameType::ENTITY_CONCRETE:
				return Webss(move(nameType.entity));
			default:
				assert(false);
			}
		}
	case Tag::NUMBER_START:
		return Webss(parseNumber(*this));
	case Tag::EXPLICIT_NAME:
		return parseOtherValueName(parseNameExplicit(it));
	case Tag::SEPARATOR:
		if (!allowVoid)
			throw runtime_error("cannot have void element");
		return Webss();
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (!allowVoid)
			throw runtime_error("cannot have void element");
		if (con.isEnd(*it))
			return Webss();
	default:
		throw runtime_error(nextTag == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}
}

Parser::OtherValue Parser::parseOtherValueName(string&& name)
{
	switch (nextTag = getTag(it))
	{
	case CASE_TAG_KEY_CHAR:
		return{ move(name), parseOtherValue().value };
	default:
		return{ move(name) };
	}
}

bool isTemplateBodyStart(Tag tag)
{
	return tag == Tag::START_TUPLE || tag == Tag::START_LIST || tag == Tag::START_DICTIONARY || tag == Tag::NAME_START;
}

Parser::OtherValue Parser::checkAbstractEntity(const Entity& ent)
{
	const auto& content = ent.getContent();
	nextTag = getTag(it);
	switch (content.getType())
	{
	case WebssType::BLOCK_HEAD:
		return{ Block(ent, parseValueOnly()) };
	case WebssType::TEMPLATE_HEAD_BINARY:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadBinary(ent), parseTemplateBodyBinary(content.getTemplateHeadBinary().getParameters())) };
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyStandard(content.getTemplateHeadStandard().getParameters())) };
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		if (isTemplateBodyStart(nextTag))
			return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyText(content.getTemplateHeadStandard().getParameters()), WebssType::TEMPLATE_TEXT) };
		break;
	default:
		break;
	}
	return{ ent };
}

void Parser::parseOtherValue(function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Entity& abstractEntity)> funcAbstractEntity)
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
		assert(false);
	}
}

void Parser::parseExplicitKeyValue(function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly)
{
	auto other = parseOtherValue(true);
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		funcKeyValue(move(other.key), move(other.value));
		break;
	case OtherValue::Type::KEY_ONLY:
		funcKeyOnly(move(other.key));
		break;
	case OtherValue::Type::VALUE_ONLY:
		throw runtime_error(ERROR_UNEXPECTED);
	default:
		assert(false);
	}
}

Webss Parser::parseValueOnly()
{
	auto otherValue = parseOtherValue();
	if (otherValue.type != OtherValue::VALUE_ONLY)
		throw runtime_error("expected value-only");
	return move(otherValue.value);
}