//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsExpand.hpp"

#include "nameType.hpp"
#include "structures/dictionary.hpp"
#include "structures/document.hpp"
#include "structures/list.hpp"
#include "structures/tuple.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

Entity webss::parseExpandEntity(SmartIterator& it, const EntityManager& ents)
{
	if (!++it || !isNameStart(*it))
		throw runtime_error("expected entity");
	auto nameType = parseNameType(it, ents);
	if (nameType.type != NameType::Type::ENTITY_CONCRETE && nameType.type != NameType::Type::ENTITY_ABSTRACT)
		throw runtime_error("expected entity");
	return move(nameType.entity);
}

const Dictionary& webss::parseExpandDictionary(SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (!ent.getContent().isDictionary())
		throw runtime_error("expand entity within dictionary must be a dictionary");
	return ent.getContent().getDictionary();
}

const List& webss::parseExpandList(SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (!ent.getContent().isList())
		throw runtime_error("expand entity within list must be a list");
	return ent.getContent().getList();
}

const Tuple& webss::parseExpandTuple(SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (!ent.getContent().isTuple())
		throw runtime_error("expand entity within tuple must be a tuple");
	return ent.getContent().getTuple();
}

void webss::expandDictionary(Dictionary& dict, SmartIterator& it, const EntityManager& ents)
{
	for (const auto& item : parseExpandDictionary(it, ents))
		dict.addSafe(item.first, item.second);
}

void webss::expandList(List& list, SmartIterator& it, const EntityManager& ents)
{
	for (const auto& item : parseExpandList(it, ents))
		list.add(item);
}

void webss::expandTuple(Tuple& tuple, SmartIterator& it, const EntityManager& ents)
{
	for (const auto& item : parseExpandTuple(it, ents).getOrderedKeyValues())
		item.first == nullptr ? tuple.add(*item.second) : tuple.addSafe(*item.first, *item.second);
}

void webss::expandNamespace(Namespace& nspace, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::NAMESPACE)
		throw runtime_error("expand entity within namespace must be a namespace");
	for (const auto& item : ent.getContent().getNamespace())
		nspace.addSafe(item);
}


void webss::expandEnum(Enum& tEnum, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::ENUM)
		throw runtime_error("expand entity within enum must be an enum");
	for (const auto& item : ent.getContent().getEnum())
		tEnum.addSafe(item.getName());
}

void webss::expandDocumentBody(Document& doc, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::TUPLE && ent.getContent().getType() != WebssType::TUPLE_TEXT)
		throw runtime_error("expand entity within document body must be a tuple");
	for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
		item.first == nullptr ? doc.add(*item.second) : doc.addSafe(*item.first, *item.second);
}