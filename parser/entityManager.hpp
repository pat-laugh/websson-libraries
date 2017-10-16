//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "structures/entity.hpp"
#include "structures/webss.hpp"

namespace webss
{
	class EntityManager
	{
	private:
		template <class T>
		struct less_ptr { bool operator()(T* t1, T* t2) const { return *t1 < *t2; } };

		std::unordered_map<std::string, Entity> ents;
	public:
		EntityManager();
		~EntityManager();

		const std::unordered_map<std::string, Entity>& getEnts() const;

		void clearAll();

		//Adds alias to entity; the name may be different than the entity's name
		void addAlias(std::string name, Entity ent);

		void add(std::string name, Webss content);
		void addSafe(std::string name, Webss content);
		void add(Entity ent);
		void addSafe(Entity ent);

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const;

		Entity& operator[](const std::string& name);
		const Entity& operator[](const std::string& name) const;

		Entity& at(const std::string& name);
		const Entity& at(const std::string& name) const;

		void remove(const std::string& name);
		void remove(const Entity& ent);
	};
}