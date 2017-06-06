//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "namespace.hpp"

namespace webss
{
	class Enum
	{
	public:
		using size_type = Namespace::size_type;
		using PtrThis = Namespace::PtrThis;
		using Namespaces = Namespace::Namespaces;
		using iterator = Namespace::iterator;
		using const_iterator = Namespace::const_iterator;

		Enum(std::string name);
		Enum(std::string name, const Namespace& previousNspace);

		bool empty() const;
		size_type size() const;

		void add(std::string key);
		void addSafe(std::string key);

		bool has(const std::string& key) const;

		bool operator==(const Enum& o) const;
		bool operator!=(const Enum& o) const;

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
		Namespace nspace;
	};
}