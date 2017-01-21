//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <map>
#include <set>
#include <unordered_map>

#include "entity.h"

namespace webss
{
	template <class T>
	struct less_ptr { bool operator()(T* t1, T* t2) const { return *t1 < *t2; } };

	template <class T>
	class BasicEntityManager
	{
	private:
		static constexpr char* ERROR_ENTITY_EXISTSs1 = "entity already exists: ";
#define ERROR_ENTITY_EXISTS(name) ERROR_ENTITY_EXISTSs1 + name
		using Entity = BasicEntity<T>;
		using Globals = std::unordered_map<std::string, Entity>;
#define name_cast const_cast<std::string*>
		Globals globals;
		std::set<std::string*, less_ptr<std::string>> locals;
		std::map<std::string*, unsigned int, less_ptr<std::string>> ordered;
		unsigned int indexEnts = 0;

		void clearOrdered()
		{
			ordered.clear();
			indexEnts = 0;
		}

		void removeOrdered(std::string* name)
		{
			ordered.erase(name);
		}

		typename Globals::iterator deleteEnt(typename Globals::iterator it)
		{
			return globals.erase(it);
		}
	public:
		BasicEntityManager() {}
		~BasicEntityManager() { clear(); }

		//globally clears all the entities
		//which means the local ent set and the global ent map are cleared
		void clear()
		{
			if (globals.empty())
				return;

			clearOrdered();
			clearLocals();

			auto it = globals.begin();
			do
				it = deleteEnt(it);
			while (it != globals.end());
		}

		//locally clears the local entities
		//which means only the local ent set is cleared
		void clearLocals() { locals.clear(); }

		//globally clears the local entities
		//which means the local ent set is cleared and all the ents that were in it are also removed from the global ent map
		void clearLocalGlobals()
		{
			while (locals.size() > 0)
				remove(**locals.begin()); //remove removes ent from locals too
		}

		//REQUIREMENT: name must not be in the globals map
		void add(std::string&& name, T&& content)
		{
			Entity ent(std::move(name), std::move(content));
			add(std::move(ent));
		}
		void addSafe(std::string&& name, T&& content)
		{
			if (hasEntity(name))
				throw std::runtime_error(ERROR_ENTITY_EXISTS(name));

			add(std::move(name), std::move(content));
		}
		void add(const std::string& name, const T& content)
		{
			Entity ent(name, content);
			add(std::move(ent));
		}
		void addSafe(const std::string& name, const T& content)
		{
			if (hasEntity(name))
				throw std::runtime_error(ERROR_ENTITY_EXISTS(name));

			add(name, content);
		}

		void add(Entity&& ent)
		{
			auto entName = const_cast<std::string*>(&ent.getName());
			ordered.insert({ entName, indexEnts++ });
			locals.insert(entName);
			globals.insert({ *entName, std::move(ent) });
		}
		void add(const Entity& ent)
		{
			auto entName = const_cast<std::string*>(&ent.getName());
			ordered.insert({ entName, indexEnts++ });
			locals.insert(entName);
			globals.insert({ *entName, ent });
		}

		//REQUIREMENT: name must be in the globals map
		void addLocals(const std::string& name)
		{
			locals.insert(name_cast(&globals.find(name)->second.getName())); //add the correct pointer from map; not from the name provided!
		}
		void addLocalsSafe(const std::string& name)
		{
			if (hasEntityLocals(name))
				throw std::runtime_error(ERROR_ENTITY_EXISTS(name));

			addLocals(name);
		}

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const { return globals.find(s) != globals.end(); }

		//returns true if s is a local entity, else false
		bool hasEntityLocals(const std::string& s) const { return locals.find(name_cast(&s)) != locals.end(); }

		//changes the name of a entity
		//REQUIREMENT: oldName must be the name of a entity and no entity with the name newName must exist
		void changeName(const std::string& oldName, std::string&& newName)
		{
			auto ptrOldName = name_cast(&oldName);

			//all elements point to the entity's name, so their pointer's value is updated automatically
			//but they still need to be removed and reinserted so that the trees balance themselves

			auto it = globals.find(oldName);
			auto ent = std::move(it->second);
			auto ptrEntName = &ent->name;

			//first erase the ent from all containers
			globals.erase(it);

			auto itOrdered = ordered.find(ptrOldName);
			auto index = itOrdered->second;
			ordered.erase(itOrdered);

			auto itLocals = locals.find(ptrOldName);
			bool localHasEnt;
			if ((localHasEnt = itLocals != locals.end()))
				locals.erase(itLocals);

			//changed ent name then insert everything back
			ent->name = std::move(newName);

			globals.insert({ *ptrEntName, std::move(ent) });
			ordered.insert({ ptrEntName, index });
			if (localHasEnt)
				locals.insert(ptrEntName);
		}

		Entity& operator[](const std::string& name) { return globals.find(name)->second; }
		const Entity& operator[](const std::string& name) const { return globals.find(name)->second; }

		Entity& at(const std::string& name) { return globals.at(name); }
		const Entity& at(const std::string& name) const { return globals.at(name); }

		std::vector<std::string*> getOrderedLocals() const
		{
			//get them ordered by index
			std::map<unsigned int, std::string*> tempOrdered;
			for (auto ent : locals)
				tempOrdered.insert({ ordered.find(ent)->second, ent });

			//then put the names in the vector
			std::vector<std::string*> orderedLocals;
			for (const auto& tempPair : tempOrdered)
				orderedLocals.push_back(tempPair.second);

			return orderedLocals;
		}

		//REQUIREMENT: entity must be in the globals map
		void remove(const std::string& name)
		{
			removeOrdered(name_cast(&name));
			if (hasEntityLocals(name))
				removeLocals(name);
			deleteEnt(globals.find(name));
		}

		//REQUIREMENT: entity must be in the locals set
		void removeLocals(const std::string& name) { locals.erase(name_cast(&name)); }

		//for use when one wants to know if the index of entities might overflow
		unsigned int getIndexEntities() { return indexEnts; }

		//for use when one thinks the index of entities might overflow
		void resetIndexEntities()
		{
			//get the entities ordered by index
			std::map<unsigned int, std::string*> tempOrdered;
			for (const auto& globalsPair : globals)
				tempOrdered.insert({ ordered.find(&globalsPair.first)->second, name_cast(&globalsPair.second.getName()) });

			//then reduce the indices as must as possible
			unsigned int minIndex = 0;
			for (const auto& tempPair : tempOrdered)
				ordered.find(tempPair.second)->second = minIndex++;

			indexEnts = minIndex;
		}
#undef ERROR_ENTITY_EXISTS
#undef name_cast
	};
}