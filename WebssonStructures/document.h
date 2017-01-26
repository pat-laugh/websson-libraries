//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <map>
#include <vector>

#include "base.h"
#include "documentHead.h"

namespace webss
{
	template <class Webss>
	class BasicDocument
	{
	public:
		using Head = BasicDocumentHead<Webss>;
		using Data = std::vector<Webss>;
		using size_type = typename Data::size_type;
		using Keymap = std::map<std::string, size_type>;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;
		using reverse_iterator = typename Data::reverse_iterator;
		using const_reverse_iterator = typename Data::const_iterator;

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		Head& getHead() { return head; }
		const Head& getHead() const { return head; }

		void add(Webss&& value) { data.push_back(std::move(value)); }
		void add(const Webss& value) { data.push_back(value); }

		void add(std::string&& key, Webss&& value)
		{
			keys.insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void add(const std::string& key, const Webss& value)
		{
			keys.insert({ key, data.size() });
			data.push_back(value);
		}

		void addSafe(std::string&& key, Webss&& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DOCUMENT + key);

			keys.insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void addSafe(const std::string& key, const Webss& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DOCUMENT + key);

			keys.insert({ key, data.size() });
			data.push_back(value);
		}

		bool has(size_type index) const { index < data.size(); }
		bool has(const std::string& key) const { return keys.find(key) != keys.end(); }

		Webss& operator[](size_type index) { return data[index]; }
		const Webss& operator[](size_type index) const { return data[index]; }
		Webss& at(size_type index) { return data.at(index); }
		const Webss& at(size_type index) const { return data.at(index); }

		Webss& operator[](const std::string& key) { return data[keys.find(key)->second]; } //find is better than [] since it does only what it's supposed to do and is as fast
		const Webss& operator[](const std::string& key) const { return data[keys.find(key)->second]; }
		Webss& at(const std::string& key) { return data.at(keys.at(key)); }
		const Webss& at(const std::string& key) const { return data.at(keys.at(key)); }

		//returns keys in order of the index they point to
		//they key of indices without an associated key are nullptr
		const std::vector<std::string*> getOrderedKeys() const
		{
			std::vector<std::string*> orderedKeys(size(), nullptr);
			for (const auto& key : keys)
				orderedKeys[key.second] = const_cast<std::string*>(&(key.first));
			return orderedKeys;
		}

		//they key of indices without an associated key are nullptr
		const std::vector<std::pair<std::string*, Webss*>> getOrderedKeyValues() const
		{
			std::vector<std::pair<std::string*, Webss*>> keyValues(size());

			auto orderedKeys = getOrderedKeys();
			for (size_type i = 0; i < size(); ++i)
			{
				keyValues[i].first = orderedKeys[i];
				keyValues[i].second = const_cast<Webss*>(&operator[](i));
			}

			return keyValues;
		}

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
		reverse_iterator rbegin() { return data.rbegin(); }
		reverse_iterator rend() { return data.rend(); }
		const_reverse_iterator rbegin() const { return data.rbegin(); }
		const_reverse_iterator rend() const { return data.rend(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_DOCUMENT = "key already in document: ";
		Keymap keys;
		Data data;

		Head head;
	};
}