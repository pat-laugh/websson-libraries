//MIT License
//Copyright 2017-2019 Patrick Laughrea
#include "parser.hpp"

#include "errors.hpp"
#include "nameType.hpp"
#include "parserNumbers.hpp"
#include "parserStrings.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

//this says to have an entity Thead Standard with a Tuple thead as if <container> and with option Plus
static const Thead FOREACH_STMT_OP(Entity("|", Thead(TheadStd(TheadStd::Params(shared_ptr<TheadStd::Params::Keymap>(new TheadStd::Params::Keymap({{"container", 0}})))), TheadOptions(false, true))));

Webss Parser::parseValueEqual()
{
	if (tagit.getSafe() == Tag::EQUAL)
		throw runtime_error(WEBSSON_EXCEPTION("expected value-only not starting with an equal sign"));
	return parseValueOnly();
}

#define CaseTagKeyChar Tag::START_DICTIONARY: case Tag::START_LIST: case Tag::START_TUPLE: \
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
		throw runtime_error(WEBSSON_EXCEPTION(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED));
	}
}

Parser::OtherValue Parser::parseOtherValue(bool explicitName)
{
	switch (tagit.getSafe())
	{
	case CaseTagKeyChar:
		return parseCharValue();
	case Tag::NAME_START:
	{
		if (explicitName)
			return parseOtherValueName(parseName(getItSafe()));
		auto nameType = parseNameType(tagit, ents);
		switch (nameType.type)
		{
		default: assert(false);
		case NameType::NAME:
			return parseOtherValueName(move(nameType.name));
		case NameType::KEYWORD:
			return Webss(nameType.keyword);
		case NameType::ENTITY_ABSTRACT:
			return checkAbstractEntity(nameType.entity);
		case NameType::ENTITY_CONCRETE:
			return Webss(move(nameType.entity));
		}
	}
	case Tag::DIGIT: case Tag::MINUS: case Tag::PLUS:
		return Webss(parseNumber(*this));
	case Tag::EXPLICIT_NAME:
		return parseOtherValueName(parseNameExplicit(tagit));
	case Tag::SEPARATOR:
		if (!allowVoid)
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_VOID));
		return Webss();
	case Tag::END_DICTIONARY: case Tag::END_LIST: case Tag::END_TUPLE: case Tag::END_TEMPLATE:
		if (!allowVoid)
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_VOID));
		if (con.isEnd(*getIt()))
			return Webss();
	case Tag::FOREACH:
	{
		++tagit;
		auto body = parseTemplateBody(FOREACH_STMT_OP);
		//TODO: verify if thead contains a iterable items (list, string, int)
		return Webss(new Webss(move(body)), WebssType::FOREACH);
	}
	case Tag::SUBSTITUTION:
		//TODO: improve this
		if (getIt().peekGood() && isDigitDec(getIt().peek()))
			return Webss(ents.at(CHAR_SUBSTITUTION + parseSubstitutionNumber(++getItSafe())));
	default:
		throw runtime_error(WEBSSON_EXCEPTION(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED));
	}
}

Parser::OtherValue Parser::parseOtherValueName(string&& name)
{
	switch (tagit.getSafe())
	{
	case CaseTagKeyChar:
		return{ move(name), parseCharValue() };
	default:
		return{ move(name) };
	}
}

//parse template body if template was only an entity
Parser::OtherValue Parser::checkAbstractEntity(const Entity& ent)
{
	const auto& content = ent.getContent();
	if (!content.isThead())
		return{ ent };
	const auto& thead = content.getThead();
	return parseTemplateBody(thead.getTypeRaw() == TypeThead::ENTITY ? thead : Thead(ent));
}

void Parser::parseOtherValue(function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Entity& abstractEntity)> funcAbstractEntity)
{
	auto other = parseOtherValue();
	switch (other.type)
	{
	default: assert(false);
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
	}
}

void Parser::parseExplicitKeyValue(function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly)
{
	auto other = parseOtherValue(true);
	switch (other.type)
	{
	default: assert(false);
	case OtherValue::Type::KEY_VALUE:
		funcKeyValue(move(other.key), move(other.value));
		break;
	case OtherValue::Type::KEY_ONLY:
		funcKeyOnly(move(other.key));
		break;
	case OtherValue::Type::VALUE_ONLY:
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	}
}

Webss Parser::parseValueOnly()
{
	auto otherValue = parseOtherValue();
	if (otherValue.type != OtherValue::VALUE_ONLY)
		throw runtime_error(WEBSSON_EXCEPTION("expected value-only"));
	return move(otherValue.value);
}