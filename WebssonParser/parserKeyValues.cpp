//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

#define PatternLineGreed(ConditionSuccess, Success, Failure) { \
if (!skipLineJunk(it)) \
	{ Failure; } \
else if (*it != '\n') \
{ \
	if (ConditionSuccess) \
		{ Success; } \
	else \
		{ Failure; } \
} \
else if (!skipJunk(++it) || !(ConditionSuccess)) \
{ \
	lineGreed = true; \
	Failure; \
} \
else \
	{ Success; } }

GlobalParser::Parser::NameType GlobalParser::Parser::parseNameType()
{
	string name = parseName(it);
	if (isKeyword(name))
		return{ Keyword(name) };
	else if (!ents.hasEntity(name))
		return{ move(name) };

	const Entity* ent = &ents[name];
scopeLoop:
	PatternLineGreed(*it == CHAR_SCOPE, /* continue below */, return{ *ent })
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

Webss GlobalParser::Parser::parseCharValue(ConType con)
{
	switch (getTag(it))
	{
	case Tag::START_DICTIONARY: ++it; return parseDictionary();
	case Tag::START_LIST: ++it; return parseList();
	case Tag::START_TUPLE: ++it; return parseTuple();
	case Tag::START_TEMPLATE: ++it; return parseTemplate(con);
	case Tag::LINE_STRING: ++it; return parseLineString(con);
	case Tag::EQUAL: ++it; return parseValueEqual(con);
	case Tag::C_STRING: ++it; return parseCString();
	case Tag::TEXT_DICTIONARY: ++it; return parseMultilineString();
	case Tag::TEXT_LIST: ++it; return{ parseListText(), true };
	case Tag::TEXT_TUPLE: ++it; return{ parseTupleText(), true };
	case Tag::TEXT_TEMPLATE: ++it; return parseTemplateText();
	default: throw runtime_error(ERROR_UNEXPECTED);
	}
}

void GlobalParser::Parser::addJsonKeyvalue(Dictionary& dict)
{
	try
	{
		skipJunkToTag(it, Tag::NAME_START);
		auto name = parseNameSafe();
		skipJunkToTag(it, Tag::C_STRING);
		if (*skipJunkToValid(++it) != CHAR_COLON)
			throw runtime_error("");
		++it;
		dict.addSafe(move(name), parseValueEqual(ConType::DICTIONARY));
	}
	catch (const runtime_error&)
	{
		throw runtime_error("could not parse supposed Json key-value");
	}
}

Webss GlobalParser::Parser::parseValueEqual(ConType con)
{
	if (*skipJunkToValid(it) != CHAR_EQUAL)
		return parseValueOnly();
	throw runtime_error("expected value-only not starting with an equal sign");
}

bool isKeyChar(char c)
{
	switch (c)
	{
	case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_TEMPLATE: case CHAR_COLON: case CHAR_EQUAL: case CHAR_CSTRING:
		return true;
	default:
		return false;
	}
}

GlobalParser::Parser::OtherValue GlobalParser::Parser::parseOtherValue(ConType con)
{
	if (isKeyChar(*it))
		return parseCharValue(con);
	else if (isNameStart(*it))
	{
		auto nameType = parseNameType();
		switch (nameType.type)
		{
		case NameType::NAME:
			PatternLineGreed(it && isKeyChar(*it), return OtherValue(move(nameType.name), parseCharValue(con)), return{ move(nameType.name) })
		case NameType::KEYWORD:
			return{ nameType.keyword };
		case NameType::ENTITY_ABSTRACT:
			return checkAbstractEntity(con, nameType.entity);
		case NameType::ENTITY_CONCRETE:
			return{ Webss(move(nameType.entity)) };
		default:
			assert(false); throw domain_error("");
		}
	}
	else if (isNumberStart(*it))
		return{ parseNumber() };
	throw runtime_error(ERROR_UNEXPECTED);
}

GlobalParser::Parser::OtherValue GlobalParser::Parser::checkAbstractEntity(ConType con, const Entity& ent)
{
	const auto& content = ent.getContent();
	switch (content.getTypeSafe())
	{
	case WebssType::BLOCK_HEAD:
		return{ Block(ent, parseValueOnly()) };
	case WebssType::TEMPLATE_HEAD_BINARY:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadBinary(ent), parseTemplateBodyBinary(content.getTemplateHeadBinarySafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_SCOPED:
		PatternLineGreed(isKeyChar(*it) || isNameStart(*it) || isNumberStart(*it), return{ TemplateScoped(ent, parseTemplateBodyScoped(ent.getContent().getTemplateHeadScopedSafe().getParameters(), con)) }, break)
	case WebssType::TEMPLATE_HEAD_STANDARD:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyStandard(content.getTemplateHeadStandardSafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_TEXT:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyText(content.getTemplateHeadStandardSafe().getParameters())) }, break)
	default:
		break;
	}
	return{ ent };
}

void GlobalParser::Parser::parseOtherValue(ConType con, std::function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Entity& abstractEntity)> funcAbstractEntity)
{
	auto other = parseOtherValue(con);
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

Webss GlobalParser::Parser::parseValueOnly()
{
	skipJunkToValid(it);
	auto otherValue = parseOtherValue(con);
	if (otherValue.type != OtherValue::VALUE_ONLY)
		throw runtime_error("expected value-only");
	return move(otherValue.value);
}