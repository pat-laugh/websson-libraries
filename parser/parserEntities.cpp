//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "errors.hpp"
#include "nameType.hpp"
#include "patternsContainers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity()
{
	auto name = parseNameExplicit(tagit);
	return Entity(move(name), parseCharValue());
}

Entity Parser::parseAbstractEntity(const Namespace& currentNamespace)
{
	auto name = parseNameExplicit(tagit);
	return Entity(name, parseAbstractCharValue(name, currentNamespace));
}

#define CASE_TAG_ABSTRACT_CHAR_VALUE Tag::START_DICTIONARY: case Tag::START_LIST: \
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE: case Tag::EQUAL: case Tag::PLUS

Webss Parser::parseAbstractCharValue(const string& name, const Namespace& currentNamespace)
{
	switch (tagit.getSafe())
	{
	case Tag::START_DICTIONARY:
		return parseNamespace(name, currentNamespace);
	case Tag::START_LIST:
		return parseEnum(name);
	case Tag::START_TEMPLATE:
		return parseTemplateHead();
	case Tag::TEXT_TEMPLATE:
		return Webss(parseTemplateHeadText(), WebssType::TEMPLATE_HEAD_TEXT);
	case Tag::EQUAL:
		++tagit;
		return parseAbstractValueEqual(name, currentNamespace);
	case Tag::PLUS:
		assert(*getIt() == CHAR_THEAD_VALUE);
		if (++tagit == Tag::START_TEMPLATE)
			return parseTemplatePlusHead();
		else if (*tagit == Tag::TEXT_TEMPLATE)
			return Webss(parseTemplatePlusHeadText(), WebssType::TEMPLATE_HEAD_PLUS_TEXT);
	default:
		throw runtime_error(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}
}

Webss Parser::parseAbstractValueEqual(const string& name, const Namespace& currentNamespace)
{
	if (tagit.getSafe() == Tag::EQUAL)
		throw runtime_error("expected abstract value-only not starting with an equal sign");
	return parseAbstractValueOnly(name, currentNamespace);
}

Webss Parser::parseAbstractValueOnly(const string& name, const Namespace& currentNamespace)
{
	switch (tagit.getSafe())
	{
	case CASE_TAG_ABSTRACT_CHAR_VALUE:
		return parseAbstractCharValue(name, currentNamespace);
	case Tag::NAME_START:
	{
		auto nameType = parseNameType(tagit, ents);
		if (nameType.type != NameType::ENTITY_ABSTRACT)
			throw runtime_error("expected abstract entity");
		return nameType.entity;
	}
	default:
		throw runtime_error(*tagit == Tag::NONE ? ERROR_EXPECTED : ERROR_UNEXPECTED);
	}	
}