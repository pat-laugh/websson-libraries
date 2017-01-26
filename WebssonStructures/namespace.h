//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <unordered_map>
#include <string>
#include <vector>

#include "base.h"
#include "entity.h"

namespace webss
{
#define This BasicNamespace
	template <class Webss>
	class This
	{
	public:
		using Entity = BasicEntity<Webss>;
		using Data = std::vector<Entity>;
		using size_type = typename Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;
		using Namespaces = std::vector<std::shared_ptr<This<Webss>>>;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;

		static const This& getEmptyInstance()
		{
			static This nspace;
			return nspace;
		}

		static This& make(std::string&& name)
		{
			std::shared_ptr<This<Webss>> ptrThis(new This(std::move(name)));
			ptrThis->nspaces.push_back(ptrThis);
			return *ptrThis;
		}
		static This& make(const std::string& name)
		{
			std::shared_ptr<This<Webss>> ptrThis(new This(name));
			ptrThis->nspaces.push_back(ptrThis);
			return *ptrThis;
		}

		static This& make(std::string&& name, const This& previousNspace)
		{
			std::shared_ptr<This<Webss>> ptrThis(new This(std::move(name), previousNspace));
			ptrThis->nspaces.push_back(ptrThis);
			return *ptrThis;
		}
		static This& make(const std::string& name, const This& previousNspace)
		{
			std::shared_ptr<This<Webss>> ptrThis(new This(name, previousNspace));
			ptrThis->nspaces.push_back(ptrThis);
			return *ptrThis;
		}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(std::string&& key, Webss&& value) { add(Entity(std::move(key), std::move(value))); }
		void add(const std::string& key, const Webss& value) { add(Entity(key, value)); }
		void addSafe(std::string&& key, Webss&& value) { addSafe(Entity(std::move(key), std::move(value))); }
		void addSafe(const std::string& key, const Webss& value) { addSafe(Entity(key, value)); }

		void add(Entity&& ent)
		{
			ent.setNamespace(getPointer());
			keys.insert({ ent.getName(), data.size() });
			data.push_back(std::move(ent));
		}
		void addSafe(Entity&& ent)
		{
			if (has(ent.getName()))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_NAMESPACE + ent.getName());

			add(std::move(ent));
		}

		bool has(const std::string& key) const { return keys.find(key) != keys.end(); }

		Entity& operator[](const std::string& key) { return data[keys.find(key)->second]; } //find is better, no key created; wonder why they thought a side-effect to a bad access would be good...
		const Entity& operator[](const std::string& key) const { return data[keys.find(key)->second]; } //[] doesn't have const and find is as fast
		Entity& at(const std::string& key) { return data[keys.at(key)]; }
		const Entity& at(const std::string& key) const { return data[keys.at(key)]; }

		const std::string& getName() const { return name; }
		const std::shared_ptr<This<Webss>>& getPointer() const { return nspaces.back(); }
		const Namespaces& getNamespaces() const { return nspaces; }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_NAMESPACE = "key already in namespace: ";

		std::string name;
		Data data;
		Keymap keys;
		Namespaces nspaces;

		This() {}

		This(std::string&& name) : name(std::move(name)) {}
		This(const std::string& name) : name(name) {}

		This(std::string&& name, const This& previousNspace) : name(std::move(name)), nspaces(previousNspace.nspaces) {}
		This(const std::string& name, const This& previousNspace) : name(name), nspaces(previousNspace.nspaces) {}
	};
#undef This
}