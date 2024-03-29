//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "utils.hpp"

namespace webss
{
	template <class T>
	class BasicSharedMap
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;
		using reverse_iterator = typename Data::reverse_iterator;
		using const_reverse_iterator = typename Data::const_reverse_iterator;

		BasicSharedMap() : keys(new Keymap()) {}
		BasicSharedMap(std::shared_ptr<Keymap>&& keys) : keys(keys), data(keys->size()) {}
		BasicSharedMap(const std::shared_ptr<Keymap>& keys) : keys(keys), data(keys->size()) {}
		BasicSharedMap(const std::shared_ptr<Keymap>& keys, const Data& data) : keys(keys), data(data) {}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(T&& value) { data.push_back(std::move(value)); }
		void add(const T& value) { data.push_back(value); }

		void add(std::string key, T value)
		{
			containerAddUnsafe(*keys, std::move(key), size());
			data.push_back(std::move(value));
		}
		void addSafe(std::string key, T value)
		{
			containerAddSafe(*keys, std::move(key), size());
			data.push_back(std::move(value));
		}

		bool has(const std::string& key) const { return keys->find(key) != keys->end(); }

		T& back()
		{
			assert(!empty());
			return data.back();
		}
		const T& back() const
		{
			assert(!empty());
			return data.back();
		}
		T& last() { return back(); }
		const T& last() const { return back(); }

		bool operator==(const BasicSharedMap& o) const { return (this == &o) || (equalPtrs(keys, o.keys) && data == o.data); }
		bool operator!=(const BasicSharedMap& o) const { return !(*this == o); }

		T& operator[](size_type index) { return accessIndexUnsafe<Data, T>(data, index); }
		const T& operator[](size_type index) const { return accessIndexUnsafe<Data, T>(data, index); }
		T& at(size_type index) { return accessIndexSafe<Data, T>(data, index); }
		const T& at(size_type index) const { return accessIndexSafe<Data, T>(data, index); }

		T& operator[](const std::string& key) { return operator[](accessKeyUnsafe<Keymap, size_type>(*keys, key)); }
		const T& operator[](const std::string& key) const { return operator[](accessKeyUnsafe<Keymap, size_type>(*keys, key)); }
		T& at(const std::string& key) { return at(accessKeySafe<Keymap, size_type>(*keys, key)); }
		const T& at(const std::string& key) const { return at(accessKeySafe<Keymap, size_type>(*keys, key)); }

		//returns keys in order of the index they point to
		//the key of indices without an associated key are nullptr
		const std::vector<std::string*> getOrderedKeys() const
		{
			std::vector<std::string*> orderedKeys(size(), nullptr);
			for (const auto& key : *keys)
				orderedKeys[key.second] = const_cast<std::string*>(&key.first);
			return orderedKeys;
		}

		//the key of indices without an associated key are nullptr
		const std::vector<std::pair<std::string*, T*>> getOrderedKeyValues() const
		{
			std::vector<std::pair<std::string*, T*>> keyValues;
			keyValues.reserve(size());

			auto orderedKeys = getOrderedKeys();
			for (size_type i = 0; i < size(); ++i)
				keyValues.push_back({ orderedKeys[i], const_cast<T*>(&operator[](i)) });

			return keyValues;
		}

		const std::shared_ptr<Keymap>& getSharedKeys() const { return keys; }

		const Data& getData() const { return data; }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
		reverse_iterator rbegin() { return data.rbegin(); }
		reverse_iterator rend() { return data.rend(); }
		const_reverse_iterator rbegin() const { return data.rbegin(); }
		const_reverse_iterator rend() const { return data.rend(); }

	protected:
		std::shared_ptr<Keymap> keys;
		Data data;

		BasicSharedMap(const Keymap& keymap, const Data& data) : keys(new Keymap(keymap)), data(data) {}
	};
}