//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <map>
#include <string>

#include "base.h"

namespace webss
{
	template<class T>
	class BasicDictionary
	{
	public:
		using Data = std::map<std::string, T>;
		using size_type = typename Data::size_type;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(std::string key, T value) { containerAddUnsafe(data, std::move(key), std::move(value)); }
		void addSafe(std::string key, T value) { containerAddSafe(data, std::move(key), std::move(value)); }

		bool has(const std::string& key) const { return data.find(key) != data.end(); }

		T& operator[](const std::string& key) { return accessKeyUnsafe<Data, T>(data, key); }
		const T& operator[](const std::string& key) const { return accessKeyUnsafe<Data, T>(data, key); }
		T& at(const std::string& key) { return accessKeySafe<Data, T>(data, key); }
		const T& at(const std::string& key) const { return accessKeySafe<Data, T>(data, key); }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
	private:
		Data data;
	};
}