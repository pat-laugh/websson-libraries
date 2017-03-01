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

Parser::NameType Parser::parseNameType(It& it)
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
		skipJunkToValidCondition(++it, [&]() { return isNameStart(*it); });
		const auto& content = ent->getContent();
		if (content.isEnum())
			return content.getEnumSafe().at(parseName(it));
		ent = &content.getNamespaceSafe().at(parseName(it));
		goto scopeLoop;
	}
	catch (const exception&) { throw runtime_error("could not get scoped value"); }
}

Webss Parser::parseCharValue(It& it, ConType con)
{
	switch (*it)
	{
	case OPEN_DICTIONARY:
		return parseDictionary(++it);
	case OPEN_LIST:
		return parseList(++it);
	case OPEN_TUPLE:
		return parseTuple(++it);
	case OPEN_TEMPLATE:
		return parseTemplate(++it, con);
	case CHAR_COLON:
		return parseValueColon(++it, con);
	case CHAR_EQUAL:
		return parseValueEqual(++it, con);
	case CHAR_CSTRING:
		return parseCString(++it);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

void Parser::addJsonKeyvalue(It& it, Dictionary& dict)
{
	try
	{
		auto name = parseNameSafe(skipJunkToValidCondition(it, [&]() { return isNameStart(*it); }));
		skipJunkToValidCondition(it, [&]() { return *it == CHAR_CSTRING; });
		skipJunkToValidCondition(++it, [&]() { return *it == CHAR_COLON; });
		dict.addSafe(move(name), parseValueEqual(++it, ConType::DICTIONARY));
	}
	catch (const exception&)
	{
		throw runtime_error("could not parse supposed Json key-value");
	}
}

Webss Parser::parseValueColon(It& it, ConType con)
{
	return it != CHAR_COLON ? parseLineString(it, con) : parseContainerText(skipJunkToValid(++it));
}

Webss Parser::parseValueEqual(It& it, ConType con)
{
	if (*skipJunkToValid(it) != CHAR_EQUAL)
		return parseValueOnly(it, con);
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

Parser::OtherValue Parser::parseOtherValue(It& it, ConType con)
{
	if (isKeyChar(*it))
		return parseCharValue(it, con);
	else if (isNameStart(*it))
	{
		auto nameType = parseNameType(it);
		switch (nameType.type)
		{
		case NameType::NAME:
			PatternLineGreed(isKeyChar(*it), return OtherValue(move(nameType.name), parseCharValue(it, con)), return{ move(nameType.name) })
		case NameType::KEYWORD:
			return{ nameType.keyword };
		case NameType::ENTITY_ABSTRACT:
			return checkAbstractEntity(it, con, nameType.entity);
		case NameType::ENTITY_CONCRETE:
			return{ Webss(move(nameType.entity)) };
		default:
			assert(false); throw domain_error("");
		}
	}
	else if (isNumberStart(*it))
		return{ parseNumber(it) };
	throw runtime_error(ERROR_UNEXPECTED);
}

Parser::OtherValue Parser::checkAbstractEntity(It& it, ConType con, const Entity& ent)
{
	const auto& content = ent.getContent();
	switch (content.getTypeSafe())
	{
	case WebssType::BLOCK_HEAD:
		return{ Block(ent, parseValueOnly(it, con)) };
	case WebssType::TEMPLATE_HEAD_BINARY:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadBinary(ent), parseTemplateBodyBinary(it, content.getTemplateHeadBinarySafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_SCOPED:
		PatternLineGreed(isKeyChar(*it) || isNameStart(*it) || isNumberStart(*it), return{ TemplateScoped(ent, parseTemplateBodyScoped(it, ent.getContent().getTemplateHeadScopedSafe().getParameters(), con)) }, break)
	case WebssType::TEMPLATE_HEAD_STANDARD:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyStandard(it, content.getTemplateHeadStandardSafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_TEXT:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyText(it, content.getTemplateHeadTextSafe().getParameters())) }, break)
	default:
		break;
	}
	return{ ent };
}

void Parser::parseOtherValue(It& it, ConType con, std::function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Entity& abstractEntity)> funcAbstractEntity)
{
	auto other = parseOtherValue(it, con);
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

Webss Parser::parseValueOnly(It& it, ConType con)
{
	auto otherValue = parseOtherValue(skipJunkToValid(it), con);
	if (otherValue.type != OtherValue::VALUE_ONLY)
		throw runtime_error("expected value-only");
	return move(otherValue.value);
}