//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "errors.hpp"
#include "patternsContainers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity()
{
	auto name = parseNameExplicit(it);
	nextTag = getTag(it);
	return Entity(move(name), parseValueOnly());
}

Entity Parser::parseAbstractEntity(const Namespace& currentNamespace)
{
	auto name = parseNameExplicit(it);
	switch (nextTag = getTag(it))
	{
	case Tag::START_DICTIONARY:
		return Entity(name, parseNamespace(name, currentNamespace));
	case Tag::START_LIST:
		return Entity(name, parseEnum(name));
	case Tag::START_TEMPLATE:
		return Entity(move(name), parseTemplateHead());
	case Tag::TEXT_TEMPLATE:
		return Entity(move(name), Webss(parseTemplateHeadText(), true));
	case Tag::EXPLICIT_NAME:
		switch (getTag(++it))
		{
		case Tag::START_DICTIONARY:
			return Entity(move(name), Webss(parseDictionary(true), true, true));
		case Tag::START_LIST:
			return Entity(move(name), Webss(parseList(true), true, true));
		case Tag::START_TUPLE:
			return Entity(move(name), Webss(parseTuple(true), true, true));
		case Tag::TEXT_LIST:
			return Entity(move(name), Webss(parseListText(true), true, true, true));
		case Tag::TEXT_TUPLE:
			return Entity(move(name), Webss(parseTupleText(true), true, true, true));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}