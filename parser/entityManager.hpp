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

		std::unordered_map<std::string, Entity> privates;
		std::set<std::string*, less_ptr<std::string>> publics;
	public:
		EntityManager();
		~EntityManager();

		std::vector<Entity> getPublicEnts() const;

		void clearAll();

		void clearPublics();

		//Adds alias to entity; the name may be different than the entity's name
		void addPrivateEntity(std::string name, Entity ent);

		void addPrivate(std::string name, Webss content);
		void addPrivateSafe(std::string name, Webss content);
		void addPrivate(Entity ent);
		void addPrivateSafe(Entity ent);

		void addPublic(std::string name, Webss content);
		void addPublicSafe(std::string name, Webss content);
		void addPublic(Entity ent);
		void addPublicSafe(Entity ent);

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const;

		Entity& operator[](const std::string& name);
		const Entity& operator[](const std::string& name) const;

		Entity& at(const std::string& name);
		const Entity& at(const std::string& name) const;

		void removePrivate(const std::string& name);
		void removePublic(const std::string& name);

		void removePrivate(const Entity& ent);
		void removePublic(const Entity& ent);
	};
}