//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "errors.hpp"
#include "nameType.hpp"
#include "patternsContainers.hpp"
#include "structures/theadFun.hpp"
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

#define CaseTagAbstractCharValue Tag::START_DICTIONARY: case Tag::START_LIST: \
	case Tag::START_TEMPLATE: case Tag::EQUAL: case Tag::PLUS

Webss Parser::parseAbstractCharValue(const string& name, const Namespace& currentNamespace)
{
	switch (tagit.getSafe())
	{
	case Tag::START_DICTIONARY:
		return parseNamespace(name, currentNamespace);
	case Tag::START_LIST:
		return parseEnum(name);
	case Tag::START_TEMPLATE:
		return parseThead();
	case Tag::START_TEMPLATE_FUN:
		return Thead(parseTheadFun());
	case Tag::EQUAL:
		++tagit;
		return parseAbstractValueEqual(name, currentNamespace);
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
	case CaseTagAbstractCharValue:
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