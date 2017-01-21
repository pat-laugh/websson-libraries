//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <unordered_map>
#include <string>
#include "entity.h"

namespace webss
{
	template <class Webss>
	class BasicNamespace
	{
	public:
		using Entity = BasicEntity<Webss>;
		using Data = std::unordered_map<std::string, Entity>;
		using size_type = typename Data::size_type;

		BasicNamespace(std::string&& name) : name(std::move(name)) {}
		BasicNamespace(const std::string& name) : name(name) {}
		~BasicNamespace() {}

		BasicNamespace(BasicNamespace&& o) : name(std::move(o.name)), data(std::move(o.data)) {}
		BasicNamespace(const BasicNamespace& o) : name(o.name), data(o.data) {}

		BasicNamespace& operator=(BasicNamespace&& o)
		{
			if (this != &o)
			{
				name = std::move(o.name);
				data = std::move(o.data);
			}
			return *this;
		}
		BasicNamespace& operator=(const BasicNamespace& o)
		{
			if (this != &o)
			{
				name = o.name;
				data = o.data;
			}
			return *this;
		}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(std::string&& key, Webss&& value)
		{
			Entity ent(std::string(key), std::move(value));
			add(std::move(ent));
		}
		void add(const std::string& key, const Webss& value)
		{
			Entity ent(key, value);
			add(std::move(ent));
		}

		void add(Entity&& ent)
		{
			data.insert({ std::string(ent.getName()), std::move(ent) });
		}
		void add(const Entity& ent)
		{
			data.insert({ ent.getName(), ent });
		}

		void addSafe(std::string&& key, Webss&& value)
		{
			Entity ent(std::move(key), std::move(value));
			addSafe(std::move(ent));
		}
		void addSafe(const std::string& key, const Webss& value)
		{
			Entity ent(key, value);
			addSafe(std::move(ent));
		}

		void addSafe(Entity&& ent)
		{
			if (has(ent.getName()))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_NAMESPACE + ent.getName());

			add(std::move(ent));
		}
		void addSafe(const Entity& ent)
		{
			if (has(ent.getName()))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_NAMESPACE + ent.getName());

			add(ent);
		}

		bool has(const std::string& key) const { return data.find(key) != data.end(); }

		Entity& operator[](const std::string& key) { return *data.find(key); } //find is better, no key created; wonder why they thought a side-effect to a bad access would be good...
		const Entity& operator[](const std::string& key) const { return *data.find(key); } //[] doesn't have const and find is as fast
		Entity& at(const std::string& key)
		{
			auto it = data.find(key);
			if (it == data.end())
				throw std::runtime_error("key is not in namespace");
			return *it;
		}
		const Entity& at(const std::string& key) const
		{
			auto it = data.find(key);
			if (it == data.end())
				throw std::runtime_error("key is not in namespace");
			return *it;
		}

		const std::string& getName() { return name; }

		typename Data::iterator begin() { return data.begin(); }
		typename Data::iterator end() { return data.end(); }
		typename Data::const_iterator begin() const { return data.begin(); }
		typename Data::const_iterator end() const { return data.end(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_NAMESPACE = "key already in namespace: ";

		std::string name;
		Data data;
	};
}