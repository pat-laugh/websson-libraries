//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "entityManager.hpp"
#include "tagIterator.hpp"
#include "structures/entity.hpp"
#include "structures/keywords.hpp"

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

	NameType parseNameType(TagIterator& tagit, const EntityManager& ents);
}