//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "namespace.h"

namespace webss
{
#define This BasicEnum
	template <class Webss>
	class This
	{
	private:
		using Namespace = BasicNamespace<Webss>;
		Namespace nspace;
	public:
		using Entity = typename Namespace::Entity;
		using size_type = typename Namespace::size_type;
		using PtrThis = typename Namespace::PtrThis;
		using Namespaces = typename Namespace::Namespaces;
		using iterator = typename Namespace::iterator;
		using const_iterator = typename Namespace::const_iterator;

		This(std::string name) : nspace(std::move(name)) {}
		This(std::string name, const This& previousNspace) : nspace(std::move(name), previousNspace) {}

		bool empty() const { return nspace.empty(); }
		size_type size() const { return nspace.size(); }

		void add(std::string key) { nspace.add(Entity(std::move(key), size())); }
		void addSafe(std::string key) { nspace.addSafe(Entity(std::move(key), size())); }

		bool has(const std::string& key) const { return nspace.has(key); }

		bool operator==(const This& o) const { return nspace == o.nspace; }

		Entity& operator[](const std::string& key) { return nspace[key]; }
		const Entity& operator[](const std::string& key) const { return nspace[key]; }
		Entity& at(const std::string& key) { return nspace.at(key); }
		const Entity& at(const std::string& key) const { return nspace.at(key); }

		const std::string& getName() const { return nspace.getName(); }
		const Namespaces& getNamespaces() const { return nspace.getNamespaces(); }
		const PtrThis& getPointer() const { return nspace.getPointer(); }

		iterator begin() { return nspace.begin(); }
		iterator end() { return nspace.end(); }
		const_iterator begin() const { return nspace.begin(); }
		const_iterator end() const { return nspace.end(); }
	};
#undef This
}