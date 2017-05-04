//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <string>
#include <unordered_map>
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
		using PtrThis = std::shared_ptr<This>;
		using Namespaces = std::vector<PtrThis>;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;

		static const This& getEmptyInstance()
		{
			static This nspace;
			return nspace;
		}

		This(std::string name) : ptrBody(new NamespaceBody{ std::move(name) })
		{
			ptrBody->ptrThis = PtrThis(new This(*this));
		}

		This(std::string name, const This& previousNspace) : ptrBody(new NamespaceBody{ std::move(name), previousNspace.getNamespaces() })
		{
			ptrBody->ptrThis = PtrThis(new This(*this));
		}

		bool empty() const
		{
			return getData().empty();
		}
		size_type size() const
		{
			return getData().size();
		}

		void add(std::string key, Webss value) { add(Entity(std::move(key), std::move(value))); }
		void addSafe(std::string key, Webss value) { addSafe(Entity(std::move(key), std::move(value))); }

		void add(Entity&& ent)
		{
			ent.setNamespace(getPointer());
			containerAddUnsafe(getKeys(), std::string(ent.getName()), size());
			getData().push_back(std::move(ent));
		}
		void addSafe(Entity&& ent)
		{
			ent.setNamespace(getPointer());
			containerAddSafe(getKeys(), std::string(ent.getName()), size());
			getData().push_back(std::move(ent));
		}

		bool has(const std::string& key) const { return getKeys().find(key) != getKeys().end(); }

		bool operator==(const This& o) const { return ptrBody == o.ptrBody; }

		Entity& operator[](const std::string& key) { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
		const Entity& operator[](const std::string& key) const { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
		Entity& at(const std::string& key) { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }
		const Entity& at(const std::string& key) const { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }

		const std::string& getName() const { return getBody().name; }
		const Namespaces& getNamespaces() const { return getBody().nspaces; }
		const PtrThis& getPointer() const { return getBody().ptrThis; }

		iterator begin() { return getData().begin(); }
		iterator end() { return getData().end(); }
		const_iterator begin() const { return getData().begin(); }
		const_iterator end() const { return getData().end(); }
	private:
		bool hasBody() const { return ptrBody.get() != nullptr; }

		struct NamespaceBody
		{
			std::string name;
			Namespaces nspaces;
			Data data;
			Keymap keys;
			PtrThis ptrThis;
		};
		std::shared_ptr<NamespaceBody> ptrBody;

		NamespaceBody& getBody() { assert(hasBody()); return *ptrBody; }
		const NamespaceBody& getBody() const  { assert(hasBody()); return *ptrBody; }
		Data& getData() { return getBody().data; }
		const Data& getData() const { return getBody().data; }
		Keymap& getKeys() { return getBody().keys; }
		const Keymap& getKeys() const { return getBody().keys; }

		This() : ptrBody(new NamespaceBody{}) {}
	};
#undef This
}