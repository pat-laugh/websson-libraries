//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "structures/entity.hpp"
#include "entityManager.hpp"
#include "utils/smartIterator.hpp"

namespace webss
{
	Entity parseExpandEntity(SmartIterator& it, const EntityManager& ents);

	void expandDictionary(Dictionary& dict, SmartIterator& it, const EntityManager& ents);
	void expandList(List& list, SmartIterator& it, const EntityManager& ents);
	void expandTuple(Tuple& tuple, SmartIterator& it, const EntityManager& ents, bool isAbstract = false);
}