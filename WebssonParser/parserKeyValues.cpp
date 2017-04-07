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
	Failure; \
} \
else \
	{ Success; } }

Parser::NameType Parser::parseNameType()
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

Parser::OtherValue Parser::parseOtherValue()
{
	if (isKeyChar(*it))
		return parseCharValue();
	else if (nextTag == Tag::NAME_START)
	{
		auto nameType = parseNameType();
		switch (nameType.type)
		{
		case NameType::NAME:
			PatternLineGreed(it && isKeyChar(*it), nextTag = getTag(it); return OtherValue(move(nameType.name), parseCharValue()), return{ move(nameType.name) })
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
	else if (nextTag == Tag::NUMBER_START)
		return parseNumber();
	else if (nextTag == Tag::EXPLICIT_NAME)
	{
		auto name = parseExplicitName();
		nextTag = getTag(it);
		return it && isKeyChar(*it) ? OtherValue(move(name), parseCharValue()) : OtherValue(name);
	}
	throw runtime_error(nextTag == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
}

Parser::OtherValue Parser::checkAbstractEntity(const Entity& ent)
{
	const auto& content = ent.getContent();
	switch (content.getTypeSafe())
	{
	case WebssType::BLOCK_HEAD:
		nextTag = getTag(it);
		return{ Block(ent, parseValueOnly()) };
	case WebssType::TEMPLATE_HEAD_BINARY:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadBinary(ent), parseTemplateBodyBinary(content.getTemplateHeadBinarySafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_SCOPED:
		PatternLineGreed(isKeyChar(*it) || isNameStart(*it) || isNumberStart(*it), return{ TemplateScoped(ent, parseTemplateBodyScoped(ent.getContent().getTemplateHeadScopedSafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_STANDARD:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyStandard(content.getTemplateHeadStandardSafe().getParameters())) }, break)
	case WebssType::TEMPLATE_HEAD_TEXT:
		PatternLineGreed(*it == OPEN_TUPLE || *it == OPEN_LIST || *it == OPEN_DICTIONARY, return{ Webss(TemplateHeadStandard(ent), parseTemplateBodyText(content.getTemplateHeadStandardSafe().getParameters())) }, break)
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