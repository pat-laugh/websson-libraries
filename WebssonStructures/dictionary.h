//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <map>

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

		void add(std::string&& key, T&& value) { data.insert({ std::move(key), std::move(value) }); }
		void add(const std::string& key, const T& value) { data.insert({ key, value }); }

		void addSafe(std::string&& key, T&& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DICTIONARY + key);

			data.insert({ std::move(key), std::move(value) });
		}
		void addSafe(const std::string& key, const T& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DICTIONARY + key);

			data.insert({ key, value });
		}

		bool has(const std::string& key) const { return data.find(key) != data.end(); }

		T& operator[](const std::string& key) { return data.find(key)->second; } //find is better, no key created; wonder why they thought a side-effect to a bad access would be good...
		const T& operator[](const std::string& key) const { return data.find(key)->second; } //[] doesn't have const and find is as fast
		T& at(const std::string& key) { return data.at(key); }
		const T& at(const std::string& key) const { return data.at(key); }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_DICTIONARY = "key already in dictionary: ";

		Data data;
	};
}