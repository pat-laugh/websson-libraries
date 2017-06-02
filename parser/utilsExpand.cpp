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

void webss::expandDictionary(Dictionary& dict, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::DICTIONARY)
		throw runtime_error("expand entity in dictionary must be a dictionary");
	for (const auto& item : ent.getContent().getDictionary())
		dict.addSafe(item.first, item.second);
}

void webss::expandList(List& list, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::LIST && ent.getContent().getType() != WebssType::LIST_TEXT)
		throw runtime_error("expand entity in list must be a list");
	for (const auto& item : ent.getContent().getList())
		list.add(item);
}

void webss::expandTuple(Tuple& tuple, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::TUPLE && ent.getContent().getType() != WebssType::TUPLE_TEXT)
		throw runtime_error("expand entity in tuple must be a tuple");
	for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
		item.first == nullptr ? tuple.add(*item.second) : tuple.addSafe(*item.first, *item.second);
}

void webss::expandNamespace(Namespace& nspace, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::NAMESPACE)
		throw runtime_error("expand entity in namespace must be a namespace");
	for (const auto& item : ent.getContent().getNamespace())
		nspace.addSafe(item);
}


void webss::expandEnum(Enum& tEnum, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::ENUM)
		throw runtime_error("expand entity in enum must be an enum");
	for (const auto& item : ent.getContent().getEnum())
		tEnum.addSafe(item.getName());
}

void webss::expandDocumentBody(Document& doc, SmartIterator& it, const EntityManager& ents)
{
	auto ent = parseExpandEntity(it, ents);
	if (ent.getContent().getType() != WebssType::TUPLE && ent.getContent().getType() != WebssType::TUPLE_TEXT)
		throw runtime_error("expand entity in document body must be a tuple");
	for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
		item.first == nullptr ? doc.add(*item.second) : doc.addSafe(*item.first, *item.second);
}