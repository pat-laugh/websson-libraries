//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

Entity GlobalParser::Parser::parseConcreteEntity(ConType con)
{
	skipJunkToTag(it, Tag::NAME_START);
	Entity ent;
	parseOtherValue(con,
		CaseKeyValue{ ent = Entity(move(key), move(value)); },
		CaseKeyOnly{ throw runtime_error(ERROR_EXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity{ throw runtime_error(webss_ERROR_ENTITY_EXISTS(abstractEntity.getName())); });
	return ent;
}

Entity GlobalParser::Parser::parseAbstractEntity(const Namespace& currentNamespace)
{
	skipJunkToTag(it, Tag::NAME_START);
	auto name = parseNameSafe();
	switch (getTag(it))
	{
	case Tag::START_DICTIONARY:
		++it;
		return Entity(move(name), parseNamespace(name, currentNamespace));
	case Tag::START_LIST:
		++it;
		return Entity(move(name), parseEnum(name));
	case Tag::START_TEMPLATE:
		++it;
		return Entity(move(name), parseTemplateHead());
	case Tag::TEXT_TEMPLATE:
		++it;
		return Entity(move(name), Webss(parseTemplateHeadText(), true));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string GlobalParser::Parser::parseNameSafe()
{
	skipJunkToTag(it, Tag::NAME_START);
	auto nameType = parseNameType();
	if (nameType.type != NameType::NAME)
		throw runtime_error("expected name that is neither an entity nor a keyword");
	return move(nameType.name);
}