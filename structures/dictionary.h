//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <map>
#include <string>

#include "webss.h"

namespace webss
{
	class Dictionary
	{
	public:
		using Data = std::map<std::string, Webss>;
		using size_type = Data::size_type;
		using iterator = Data::iterator;
		using const_iterator = Data::const_iterator;

		bool empty() const;
		size_type size() const;

		void add(std::string key, Webss value);
		void addSafe(std::string key, Webss value);

		bool has(const std::string& key) const;

		Webss& operator[](const std::string& key);
		const Webss& operator[](const std::string& key) const;
		Webss& at(const std::string& key);
		const Webss& at(const std::string& key) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
	private:
		Data data;
	};
}