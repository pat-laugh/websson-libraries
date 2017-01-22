//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity(It& it, ConType con)
{
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
	auto name = parseNameSafe(it);
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY:
		return Entity(move(name), parseNamespace(++it, name, currentNamespace));
	case OPEN_LIST:
		return Entity(move(name), Webss(parseEnum(++it, name), true));
	case OPEN_FUNCTION:
		return Entity(move(name), parseFunctionHead(++it));
	case CHAR_COLON:
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		return Entity(move(name), parseFunctionHeadText(++it));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string Parser::parseName(It& it)
{
	string name;
	do
		name += *it;
	while (++it && isNameBody(*it));
	return name;
}

string Parser::parseNameSafe(It& it)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	auto nameType = parseNameType(it);
	if (nameType.type == NameType::NAME)
		return move(nameType.name);
	throw runtime_error("expected name that is neither an entity nor a keyword");
}

void Parser::parseUsingNamespace(It& it, function<void(const Entity& ent)> funcForEach)
{
	if (*skipJunkToValid(it) == OPEN_FUNCTION)
		//....
		return;

	parseOtherValue(it, ConType::DOCUMENT,
		CaseKeyValue{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseKeyOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity
		{
			for (const auto& entPair : checkIsNamespace(abstractEntity))
				funcForEach(entPair.second);
		});
}