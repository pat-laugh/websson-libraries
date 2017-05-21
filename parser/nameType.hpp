//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "structures/entity.hpp"
#include "entityManager.hpp"
#include "structures/keywords.hpp"
#include "utils/smartIterator.hpp"

namespace webss
{
	class NameType
	{
	public:
		enum Type { NAME, KEYWORD, ENTITY_ABSTRACT, ENTITY_CONCRETE };

		NameType(std::string&& name);
		NameType(Keyword keyword);
		NameType(const Entity& entity);

		Type type;
		std::string name;
		Keyword keyword;
		Entity entity;
	};

	NameType parseNameType(SmartIterator& it, const EntityManager& ents);
}