//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <set>
#include <string>
#include "variable.h"

namespace webss
{
	template <class Webss>
	class BasicNamespace
	{
	public:
		using Variable = BasicVariable<Webss>;
	private:
		struct less_ptr { bool operator()(const Variable& t1, const Variable& t2) const { return t1.getName() < t2.getName(); } };
	public:
		using Data = std::set<Variable, less_ptr>;
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
			Variable var(std::move(key), std::move(value));
			add(std::move(var));
		}
		void add(const std::string& key, const Webss& value)
		{
			Variable var(key, value);
			add(std::move(var));
		}

		void add(Variable&& var)
		{
			data.insert(std::move(var));
		}
		void add(const Variable& var)
		{
			data.insert(var);
		}

		void addSafe(Variable&& var)
		{
			if (has(var.getName()))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_NAMESPACE + var.getName());

			add(std::move(var));
		}
		void addSafe(const Variable& var)
		{
			if (has(var.getName()))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_NAMESPACE + var.getName());

			add(var);
		}

		void addSafe(std::string&& key, Webss&& value)
		{
			Variable var(std::move(key), std::move(value));
			addSafe(std::move(var));
		}
		void addSafe(const std::string& key, const Webss& value)
		{
			Variable var(key, value);
			addSafe(std::move(var));
		}

		bool has(const std::string& key) const { return data.find(Variable(key, Webss())) != data.end(); }

		Variable& operator[](const std::string& key) { return *data.find(Variable(key, Webss())); } //find is better, no key created; wonder why they thought a side-effect to a bad access would be good...
		const Variable& operator[](const std::string& key) const { return *data.find(Variable(key, Webss())); } //[] doesn't have const and find is as fast
		Variable& at(const std::string& key)
		{
			auto it = data.find(Variable(key, Webss()));
			if (it == data.end())
				throw std::runtime_error("key is not in namespace");
			return *it;
		}
		const Variable& at(const std::string& key) const
		{
			auto it = data.find(Variable(key, Webss()));
			if (it == data.end())
				throw std::runtime_error("key is not in namespace");
			return *it;
		}

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