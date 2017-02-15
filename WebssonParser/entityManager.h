//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <set>
#include <unordered_map>

#include "WebssonStructures/entity.h"

namespace webss
{
	template <class T>
	struct less_ptr { bool operator()(T* t1, T* t2) const { return *t1 < *t2; } };

	template <class T>
	class BasicEntityManager
	{
	private:
		static constexpr char* ERROR_ENTITY_EXISTS = "entity already exists: ";
		using Entity = BasicEntity<T>;
		std::unordered_map<std::string, Entity> globals;
		std::set<std::string*, less_ptr<std::string>> locals;
	public:
		BasicEntityManager() {}
		~BasicEntityManager() { clearAll(); }

		void clearAll()
		{
			clearLocals();
			for (auto it = globals.begin(); it != globals.end();)
				it = globals.erase(it);
		}

		void clearLocals()
		{
			for (auto it = locals.begin(); it != locals.end();)
				it = locals.erase(it);
		}

		void addGlobal(std::string name, T content)
		{
			addGlobal(Entity(std::move(name), std::move(content)));
		}
		void addGlobalSafe(std::string name, T content)
		{
			addGlobalSafe(Entity(std::move(name), std::move(content)));
		}
		void addGlobal(Entity ent)
		{
			const auto& name = ent.getName();
			globals.insert({ name, std::move(ent) });
		}
		void addGlobalSafe(Entity ent)
		{
			if (hasEntity(ent.getName()))
				throw std::runtime_error(ERROR_ENTITY_EXISTS + name);

			addGlobal(std::move(ent));
		}

		void addLocal(std::string name, T content)
		{
			addLocal(Entity(std::move(name), std::move(content)));
		}
		void addLocalSafe(std::string name, T content)
		{
			addLocalSafe(Entity(std::move(name), std::move(content)));
		}
		void addLocal(Entity ent)
		{
			locals.insert(const_cast<std::string*>(&ent.getName()));
			addGlobal(std::move(ent));
		}
		void addLocalSafe(Entity ent)
		{
			if (hasEntity(ent.getName()))
				throw std::runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

			addLocal(std::move(ent));
		}

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const { return globals.find(s) != globals.end(); }

		Entity& operator[](const std::string& name) { return globals.find(name)->second; }
		const Entity& operator[](const std::string& name) const { return globals.find(name)->second; }

		Entity& at(const std::string& name) { return globals.at(name); }
		const Entity& at(const std::string& name) const { return globals.at(name); }

		void removeGlobal(const std::string& name)
		{
			globals.erase(name);
		}
		void removeLocal(const std::string& name)
		{
			locals.erase(const_cast<std::string*>(&name));
			removeGlobal(name);
		}

		void removeGlobal(const Entity& ent)
		{
			removeGlobal(ent.getName());
		}
		void removeLocal(const Entity& ent)
		{
			removeLocal(ent.getName());
		}
	};
}