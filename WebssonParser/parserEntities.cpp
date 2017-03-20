//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

Entity GlobalParser::Parser::parseConcreteEntity(It& it, ConType con)
{
	skipJunkToTag(it, Tag::NAME_START);
	Entity ent;
	parseOtherValue(it, con,
		CaseKeyValue{ ent = Entity(move(key), move(value)); },
		CaseKeyOnly{ throw runtime_error(ERROR_EXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity{ throw runtime_error(webss_ERROR_ENTITY_EXISTS(abstractEntity.getName())); });
	return ent;
}

Entity GlobalParser::Parser::parseAbstractEntity(It& it, const Namespace& currentNamespace)
{
	auto name = parseNameSafe(skipJunkToTag(it, Tag::NAME_START));
	switch (getTag(it))
	{
	case Tag::START_DICTIONARY:
		return Entity(move(name), parseNamespace(++it, name, currentNamespace));
	case Tag::START_LIST:
		return Entity(move(name), parseEnum(++it, name));
	case Tag::START_TEMPLATE:
		return Entity(move(name), parseTemplateHead(++it));
	case Tag::TEXT_TEMPLATE:
		return Entity(move(name), Webss(parseTemplateHeadText(++it), true));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string GlobalParser::Parser::parseNameSafe(It& it)
{
	auto nameType = parseNameType(skipJunkToTag(it, Tag::NAME_START));
	if (nameType.type != NameType::NAME)
		throw runtime_error("expected name that is neither an entity nor a keyword");
	return move(nameType.name);
}