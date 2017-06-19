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
	if (tagit.getSafe() == Tag::EQUAL)
		throw runtime_error("expected value-only not starting with an equal sign");
	return parseValueOnly();
}

#define CASE_TAG_KEY_CHAR Tag::START_DICTIONARY: case Tag::START_LIST: case Tag::START_TUPLE: \
	case Tag::START_TEMPLATE: case Tag::LINE_STRING: case Tag::EQUAL: case Tag::C_STRING: \
	case Tag::TEXT_DICTIONARY: case Tag::TEXT_LIST: case Tag::TEXT_TUPLE

Webss Parser::parseCharValue()
{
	switch (tagit.getSafe())
	{
	case Tag::START_DICTIONARY: return parseDictionary();
	case Tag::START_LIST: return parseList();
	case Tag::START_TUPLE: return parseTuple();
	case Tag::START_TEMPLATE: return parseTemplate();
	case Tag::LINE_STRING: ++getIt(); return parseLineString(*this);
	case Tag::EQUAL: ++tagit; return parseValueEqual();
	case Tag::C_STRING: return parseCString(*this);
	case Tag::TEXT_DICTIONARY: return parseMultilineString(*this);
	case Tag::TEXT_LIST: return{ parseListText(), WebssType::LIST_TEXT };
	case Tag::TEXT_TUPLE: return{ parseTupleText(), WebssType::TUPLE_TEXT };
	default:
		throw runtime_error(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}
}

Parser::OtherValue Parser::parseOtherValue(bool explicitName)
{
	switch (tagit.getSafe())
	{
	case CASE_TAG_KEY_CHAR:
		return parseCharValue();
	case Tag::NAME_START:
		if (explicitName)
			return parseOtherValueName(parseName(getItSafe()));
		else
		{
			auto nameType = parseNameType(tagit, ents);
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
	case Tag::DIGIT: case Tag::MINUS: case Tag::PLUS:
		return Webss(parseNumber(*this));
	case Tag::EXPLICIT_NAME:
		return parseOtherValueName(parseNameExplicit(tagit));
	case Tag::SEPARATOR:
		if (!allowVoid)
			throw runtime_error("cannot have void element");
		return Webss();
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (!allowVoid)
			throw runtime_error("cannot have void element");
		if (con.isEnd(*getIt()))
			return Webss();
	default:
		throw runtime_error(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}
}

Parser::OtherValue Parser::parseOtherValueName(string&& name)
{
	switch (tagit.getSafe())
	{
	case CASE_TAG_KEY_CHAR:
		return{ move(name), parseCharValue() };
	default:
		return{ move(name) };
	}
}

Parser::OtherValue Parser::checkAbstractEntity(const Entity& ent)
{
	const auto& content = ent.getContent();
	if (!content.isThead())
		return{ ent };
	const auto& thead = content.getThead();
	switch (thead.getTypeRaw())
	{
	case TypeThead::BIN:
		return parseTemplateBin(Thead(ent));
	case TypeThead::STD:
		return parseTemplateStd(Thead(ent));
	case TypeThead::ENTITY:
		assert(thead.isTheadStd() || thead.isTheadBin());
		return thead.isTheadBin() ? parseTemplateBin(thead) : parseTemplateStd(thead);
	default:
		assert(false); throw domain_error("");
	}
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