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
	return Entity(move(name), parseCharValue());
}

Entity Parser::parseAbstractEntity(const Namespace& currentNamespace)
{
	auto name = parseNameExplicit(it);
	switch (getTag(it))
	{
	case Tag::START_DICTIONARY:
		return Entity(name, parseNamespace(name, currentNamespace));
	case Tag::START_LIST:
		return Entity(name, parseEnum(name));
	case Tag::START_TEMPLATE:
		return Entity(move(name), parseTemplateHead());
	case Tag::TEXT_TEMPLATE:
		return Entity(move(name), Webss(parseTemplateHeadText(), WebssType::TEMPLATE_HEAD_TEXT));
	case Tag::PLUS:
		assert(*it == CHAR_THEAD_VALUE);
		switch (getTag(++it))
		{
		case Tag::START_TEMPLATE:
			return Entity(move(name), parseTemplateValueHead());
		case Tag::TEXT_TEMPLATE:
			return Entity(move(name), Webss(parseTemplateValueHeadText(), WebssType::TEMPLATE_VALUE_HEAD_TEXT));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}