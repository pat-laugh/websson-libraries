//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsExpand.hpp"

#include "errors.hpp"
#include "nameType.hpp"
#include "utilsSweepers.hpp"
#include "structures/dictionary.hpp"
#include "structures/document.hpp"
#include "structures/list.hpp"
#include "structures/tuple.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Entity webss::parseExpandEntity(TagIterator& tagit, const EntityManager& ents)
{
	checkNameExplicit(tagit);
	auto nameType = parseNameType(tagit, ents);
	if (nameType.type != NameType::Type::ENTITY_CONCRETE && nameType.type != NameType::Type::ENTITY_ABSTRACT)
		throw runtime_error(WEBSSON_EXCEPTION("expected entity"));
	return move(nameType.entity);
}

const Dictionary& webss::parseExpandDictionary(TagIterator& tagit, const EntityManager& ents)
{
	auto ent = parseExpandEntity(tagit, ents);
	if (!ent.getContent().isDictionary())
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within dictionary must be a dictionary"));
	return ent.getContent().getDictionary();
}

const List& webss::parseExpandList(TagIterator& tagit, const EntityManager& ents)
{
	auto ent = parseExpandEntity(tagit, ents);
	if (!ent.getContent().isList())
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within list must be a list"));
	return ent.getContent().getList();
}

const Tuple& webss::parseExpandTuple(TagIterator& tagit, const EntityManager& ents)
{
	auto ent = parseExpandEntity(tagit, ents);
	if (!ent.getContent().isTuple())
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within tuple must be a tuple"));
	return ent.getContent().getTuple();
}

void webss::expandDictionary(Dictionary& dict, TagIterator& tagit, const EntityManager& ents)
{
	for (const auto& item : parseExpandDictionary(tagit, ents))
		dict.addSafe(item.first, item.second);
}

void webss::expandList(List& list, TagIterator& tagit, const EntityManager& ents)
{
	for (const auto& item : parseExpandList(tagit, ents))
		list.add(item);
}

void webss::expandTuple(Tuple& tuple, TagIterator& tagit, const EntityManager& ents)
{
	for (const auto& item : parseExpandTuple(tagit, ents).getOrderedKeyValues())
		item.first == nullptr ? tuple.add(*item.second) : tuple.addSafe(*item.first, *item.second);
}

void webss::expandNamespace(Namespace& nspace, TagIterator& tagit, const EntityManager& ents)
{
	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::NAMESPACE)
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within namespace must be a namespace"));
	for (const auto& item : ent.getContent().getNamespace())
		nspace.addSafe(item);
}

void webss::expandEnum(Enum& tEnum, TagIterator& tagit, const EntityManager& ents)
{
	auto ent = parseExpandEntity(tagit, ents);
	if (ent.getContent().getType() != WebssType::ENUM)
		throw runtime_error(WEBSSON_EXCEPTION("expand entity within enum must be an enum"));
	for (const auto& item : ent.getContent().getEnum())
		tEnum.addSafe(item.getName());
}