//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <set>
#include <unordered_map>
#include <vector>

#include "structures/entity.h"
#include "structures/webss.h"

namespace webss
{
	class EntityManager
	{
	private:
		template <class T>
		struct less_ptr { bool operator()(T* t1, T* t2) const { return *t1 < *t2; } };

		std::unordered_map<std::string, Entity> globals;
		std::set<std::string*, less_ptr<std::string>> locals;
	public:
		EntityManager();
		~EntityManager();

		std::vector<Entity> getLocalEnts() const;

		void clearAll();

		void clearLocals();

		//Adds alias to entity; the name may be different than the entity's name
		void addGlobalEntity(std::string name, Entity ent);

		void addGlobal(std::string name, Webss content);
		void addGlobalSafe(std::string name, Webss content);
		void addGlobal(Entity ent);
		void addGlobalSafe(Entity ent);

		void addLocal(std::string name, Webss content);
		void addLocalSafe(std::string name, Webss content);
		void addLocal(Entity ent);
		void addLocalSafe(Entity ent);

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const;

		Entity& operator[](const std::string& name);
		const Entity& operator[](const std::string& name) const;

		Entity& at(const std::string& name);
		const Entity& at(const std::string& name) const;

		void removeGlobal(const std::string& name);
		void removeLocal(const std::string& name);

		void removeGlobal(const Entity& ent);
		void removeLocal(const Entity& ent);
	};
}