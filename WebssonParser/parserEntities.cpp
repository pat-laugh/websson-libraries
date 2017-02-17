//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity(It& it, ConType con)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	Entity ent;
	parseOtherValue(it, con,
		CaseKeyValue{ ent = Entity(move(key), move(value)); },
		CaseKeyOnly{ throw runtime_error(ERROR_EXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity{ throw runtime_error(webss_ERROR_ENTITY_EXISTS(abstractEntity.getName())); });
	return ent;
}

Entity Parser::parseAbstractEntity(It& it, const Namespace& currentNamespace)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	auto name = parseNameSafe(it);
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY:
		return Entity(move(name), parseNamespace(++it, name, currentNamespace));
	case OPEN_LIST:
		return Entity(move(name), parseEnum(++it, name));
	case OPEN_TEMPLATE:
		return Entity(move(name), parseTemplateHead(++it));
	case CHAR_COLON:
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_TEMPLATE)
			throw runtime_error("expected text template head");
		return Entity(move(name), parseTemplateHeadText(++it));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string Parser::parseNameSafe(It& it)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	auto nameType = parseNameType(it);
	if (nameType.type != NameType::NAME)
		throw runtime_error("expected name that is neither an entity nor a keyword");
	return move(nameType.name);
}