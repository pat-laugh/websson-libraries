//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <map>
#include <set>

#include "variable.h"
#include "lessPtr.h"

namespace webss
{
	template <class T>
	class BasicVariablesManager
	{
	private:
		static constexpr char* ERROR_VARIABLE_EXISTSs1 = "variable already exists: ";
#define ERROR_VARIABLE_EXISTS(name) ERROR_VARIABLE_EXISTSs1 + name
		using Variable = BasicVariable<T>;
		using Globals = std::map<std::string*, Variable, less_ptr<std::string>>;
#define name_cast const_cast<std::string*>
		Globals globals;
		std::set<std::string*, less_ptr<std::string>> locals;
		std::map<std::string*, unsigned int, less_ptr<std::string>> ordered;
		unsigned int indexVars = 0;

		void clearOrdered()
		{
			ordered.clear();
			indexVars = 0;
		}

		void removeOrdered(std::string* name)
		{
			ordered.erase(name);
		}

		typename Globals::iterator deleteVar(typename Globals::iterator it)
		{
			return globals.erase(it);
		}
	public:
		BasicVariablesManager() {}
		~BasicVariablesManager() { clear(); }

		//globally clears all the variables
		//which means the local var set and the global var map are cleared
		void clear()
		{
			if (globals.empty())
				return;

			clearOrdered();
			clearLocals();

			auto it = globals.begin();
			do
				it = deleteVar(it);
			while (it != globals.end());
		}

		//locally clears the local variables
		//which means only the local var set is cleared
		void clearLocals() { locals.clear(); }

		//globally clears the local variables
		//which means the local var set is cleared and all the vars that were in it are also removed from the global var map
		void clearLocalGlobals()
		{
			while (locals.size() > 0)
				remove(**locals.begin()); //remove removes var from locals too
		}

		//REQUIREMENT: name must not be in the globals map
		void add(std::string&& name, T&& content)
		{
			Variable var(std::move(name), std::move(content));
			add(std::move(var));
		}
		void addSafe(std::string&& name, T&& content)
		{
			if (hasVariable(name))
				throw std::runtime_error(ERROR_VARIABLE_EXISTS(name));

			add(std::move(name), std::move(content));
		}
		void add(const std::string& name, const T& content)
		{
			Variable var(name, content);
			add(std::move(var));
		}
		void addSafe(const std::string& name, const T& content)
		{
			if (hasVariable(name))
				throw std::runtime_error(ERROR_VARIABLE_EXISTS(name));

			add(name, content);
		}

		void add(Variable&& var)
		{
			auto varName = const_cast<std::string*>(&var.getName());
			ordered.insert({ varName, indexVars++ });
			locals.insert(varName);
			globals.insert({ varName, std::move(var) });
		}
		void add(const Variable& var)
		{
			auto varName = const_cast<std::string*>(&var.getName());
			ordered.insert({ varName, indexVars++ });
			locals.insert(varName);
			globals.insert({ varName, var });
		}

		//REQUIREMENT: name must be in the globals map
		void addLocals(const std::string& name)
		{
			locals.insert(globals.find(name_cast(&name))->first); //add the correct pointer from map; not from the name provided!
		}
		void addLocalsSafe(const std::string& name)
		{
			if (hasVariableLocals(name))
				throw std::runtime_error(ERROR_VARIABLE_EXISTS(name));

			addLocals(name);
		}

		//returns true if s is a variable, else false
		bool hasVariable(const std::string& s) const { return globals.find(name_cast(&s)) != globals.end(); }

		//returns true if s is a local variable, else false
		bool hasVariableLocals(const std::string& s) const { return locals.find(name_cast(&s)) != locals.end(); }

		//changes the name of a variable
		//REQUIREMENT: oldName must be the name of a variable and no variable with the name newName must exist
		void changeName(const std::string& oldName, std::string&& newName)
		{
			auto oldName_p = name_cast(&oldName);

			//all elements point to the variable's name, so their pointer's value is updated automatically
			//but they still need to be removed and reinserted so that the trees balance themselves

			auto it = globals.find(oldName_p);
			auto var = std::move(it->second);
			auto varName_p = &var->name;

			//first erase the var from all containers
			globals.erase(it);

			auto itOrdered = ordered.find(oldName_p);
			auto index = itOrdered->second;
			ordered.erase(itOrdered);

			auto itLocals = locals.find(oldName_p);
			bool localHasVar;
			if ((localHasVar = itLocals != locals.end()))
				locals.erase(itLocals);

			//changed var name then insert everything back
			var->name = std::move(newName);

			globals.insert({ varName_p, std::move(var) });
			ordered.insert({ varName_p, index });
			if (localHasVar)
				locals.insert(varName_p);
		}

		Variable& operator[](const std::string& name) { return globals.find(name_cast(&name))->second; }
		const Variable& operator[](const std::string& name) const { return globals.find(name_cast(&name))->second; }

		Variable& at(const std::string& name) { return globals.at(name_cast(&name)); }
		const Variable& at(const std::string& name) const { return globals.at(name_cast(&name)); }

		std::vector<std::string*> getOrderedLocals() const
		{
			//get them ordered by index
			std::map<unsigned int, std::string*> tempOrdered;
			for (auto var : locals)
				tempOrdered.insert({ ordered.find(var)->second, var });

			//then put the names in the vector
			std::vector<std::string*> orderedLocals;
			for (const auto& tempPair : tempOrdered)
				orderedLocals.push_back(tempPair.second);

			return orderedLocals;
		}

		//REQUIREMENT: variable must be in the globals map
		void remove(const std::string& name)
		{
			removeOrdered(name_cast(&name));
			if (hasVariableLocals(name))
				removeLocals(name);
			deleteVar(globals.find(name_cast(&name)));
		}

		//REQUIREMENT: variable must be in the locals set
		void removeLocals(const std::string& name) { locals.erase(name_cast(&name)); }

		//for use when one wants to know if the index of variables might overflow
		unsigned int getIndexVariables() { return indexVars; }

		//for use when one thinks the index of variables might overflow
		void resetIndexVariables()
		{
			//get the variables ordered by index
			std::map<unsigned int, std::string*> tempOrdered;
			for (const auto& globalsPair : globals)
				tempOrdered.insert({ ordered.find(globalsPair.first)->second, globalsPair.first });

			//then reduce the indices as must as possible
			unsigned int minIndex = 0;
			for (const auto& tempPair : tempOrdered)
				ordered.find(tempPair.second)->second = minIndex++;

			indexVars = minIndex;
		}
#undef ERROR_VARIABLE_EXISTS
#undef name_cast
	};
}