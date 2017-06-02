//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base.hpp"

namespace webss
{
	class Namespace
	{
	public:
		using Data = std::vector<Entity>;
		using size_type = Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;
		using PtrThis = std::shared_ptr<Namespace>;
		using Namespaces = std::vector<PtrThis>;
		using iterator = Data::iterator;
		using const_iterator = Data::const_iterator;

		static const Namespace& getEmptyInstance()
		{
			static Namespace nspace;
			return nspace;
		}

		Namespace(std::string name);

		Namespace(std::string name, const Namespace& previousNspace);

		bool empty() const;
		size_type size() const;

		void add(std::string key, Webss value);
		void addSafe(std::string key, Webss value);

		void add(Entity ent);
		void addSafe(Entity ent);

		bool has(const std::string& key) const;

		bool operator==(const Namespace& o) const;

		Entity& operator[](const std::string& key);
		const Entity& operator[](const std::string& key) const;
		Entity& at(const std::string& key);
		const Entity& at(const std::string& key) const;

		const std::string& getName() const;
		const Namespaces& getNamespaces() const;
		const PtrThis& getPointer() const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
	private:
		bool hasBody() const;

		struct NamespaceBody;
		std::shared_ptr<NamespaceBody> ptrBody;

		NamespaceBody& getBody();
		const NamespaceBody& getBody() const;
		Data& getData();
		const Data& getData() const;
		Keymap& getKeys();
		const Keymap& getKeys() const;

		Namespace();
	};
}