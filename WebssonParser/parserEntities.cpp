//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity()
{
	auto name = parseNameExplicit();
	nextTag = getTag(it);
	return Entity(move(name), parseValueOnly());
}

Entity Parser::parseAbstractEntity(const Namespace& currentNamespace)
{
	auto name = parseNameExplicit();
	switch (nextTag = getTag(it))
	{
	case Tag::START_DICTIONARY:
		return Entity(move(name), parseNamespace(name, currentNamespace));
	case Tag::START_LIST:
		return Entity(move(name), parseEnum(name));
	case Tag::START_TEMPLATE:
		return Entity(move(name), parseTemplateHead());
	case Tag::TEXT_TEMPLATE:
		return Entity(move(name), Webss(parseTemplateHeadText(), true));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string Parser::parseNameSafe()
{
	skipJunkToTag(it, Tag::NAME_START);
	auto nameType = parseNameType();
	if (nameType.type != NameType::NAME)
		throw runtime_error("expected name that is neither an entity nor a keyword");
	return move(nameType.name);
}

string Parser::parseNameExplicit()
{
	skipJunkToTag(++it, Tag::NAME_START);
	return parseName(it);
}