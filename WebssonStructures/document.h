//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace webss
{
	template <class T>
	class BasicDocument
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;

		BasicDocument() {}
		~BasicDocument() {}

		BasicDocument(BasicDocument&& o) : keys(std::move(o.keys)), data(std::move(o.data)) {}
		BasicDocument(const BasicDocument& o) : keys(o.keys), data(o.data) {}

		BasicDocument& operator=(BasicDocument&& o)
		{
			if (this != &o)
			{
				keys = std::move(o.keys);
				data = std::move(o.data);
			}
			return *this;
		}
		BasicDocument& operator=(const BasicDocument& o)
		{
			if (this != &o)
			{
				keys = o.keys;
				data = o.data;
			}
			return *this;
		}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(T&& value) { data.push_back(std::move(value)); }
		void add(const T& value) { data.push_back(value); }

		void add(std::string&& key, T&& value)
		{
			keys.insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void add(const std::string& key, const T& value)
		{
			keys.insert({ key, data.size() });
			data.push_back(value);
		}

		void addSafe(std::string&& key, T&& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DOCUMENT + key);

			keys.insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void addSafe(const std::string& key, const T& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_DOCUMENT + key);

			keys.insert({ key, data.size() });
			data.push_back(value);
		}

		bool has(size_type index) const { index < data.size(); }
		bool has(const std::string& key) const { return keys.find(key) != keys.end(); }

		T& operator[](size_type index) { return data[index]; }
		const T& operator[](size_type index) const { return data[index]; }
		T& at(size_type index) { return data.at(index); }
		const T& at(size_type index) const { return data.at(index); }

		T& operator[](const std::string& key) { return data[keys.find(key)->second]; } //find is better than [] since it does only what it's supposed to do and is as fast
		const T& operator[](const std::string& key) const { return data[keys.find(key)->second]; }
		T& at(const std::string& key) { return data.at(keys.at(key)); }
		const T& at(const std::string& key) const { return data.at(keys.at(key)); }

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
		const std::vector<std::pair<std::string*, T*>> getOrderedKeyValues() const
		{
			std::vector<std::pair<std::string*, T*>> keyValues(size());

			auto orderedKeys = getOrderedKeys();
			for (size_type i = 0; i < size(); ++i)
			{
				keyValues[i].first = orderedKeys[i];
				keyValues[i].second = const_cast<T*>(&operator[](i));
			}

			return keyValues;
		}

		typename Data::iterator begin() { return data.begin(); }
		typename Data::iterator end() { return data.end(); }
		typename Data::const_iterator begin() const { return data.begin(); }
		typename Data::const_iterator end() const { return data.end(); }
		typename Data::reverse_iterator rbegin() { return data.rbegin(); }
		typename Data::reverse_iterator rend() { return data.rend(); }
		typename Data::const_reverse_iterator rbegin() const { return data.rbegin(); }
		typename Data::const_reverse_iterator rend() const { return data.rend(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_DOCUMENT = "key already in document: ";
		Keymap keys;
		Data data;
	};
}