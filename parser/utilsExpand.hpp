//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "entityManager.hpp"
#include "tagIterator.hpp"
#include "structures/entity.hpp"

namespace webss
{
	Entity parseExpandEntity(TagIterator& tagit, const EntityManager& ents);
	const Dictionary& parseExpandDictionary(TagIterator& tagit, const EntityManager& ents);
	const List& parseExpandList(TagIterator& tagit, const EntityManager& ents);
	const Tuple& parseExpandTuple(TagIterator& tagit, const EntityManager& ents);

	void expandDictionary(Dictionary& dict, TagIterator& tagit, const EntityManager& ents);
	void expandList(List& list, TagIterator& tagit, const EntityManager& ents);
	void expandTuple(Tuple& tuple, TagIterator& tagit, const EntityManager& ents);
	void expandNamespace(Namespace& nspace, TagIterator& tagit, const EntityManager& ents);
	void expandEnum(Enum& tEnum, TagIterator& tagit, const EntityManager& ents);
}