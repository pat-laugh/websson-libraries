//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <map>
#include <string>
#include "variable.h"
#include "lessPtr.h"

namespace webss
{
	template <class Webss>
	class BasicNamespace
	{
	public:
		using Variable = BasicVariable<Webss>;
		using Data = std::map<std::string*, Variable, less_ptr<std::string>>;
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
			auto varName = const_cast<std::string*>(&var.getName());
			data.insert({ varName, std::move(var) });
		}
		void add(const Variable& var)
		{
			auto varName = const_cast<std::string*>(&var.getName());
			data.insert({ varName, var });
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

		bool has(const std::string& key) const { return data.find(const_cast<std::string*>(&key)) != data.end(); }

		Variable& operator[](const std::string& key) { return data.find(const_cast<std::string*>(&key))->second; } //find is better, no key created; wonder why they thought a side-effect to a bad access would be good...
		const Variable& operator[](const std::string& key) const { return data.find(const_cast<std::string*>(&key))->second; } //[] doesn't have const and find is as fast
		Variable& at(const std::string& key) { return data.at(const_cast<std::string*>(&key)); }
		const Variable& at(const std::string& key) const { return data.at(const_cast<std::string*>(&key)); }

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