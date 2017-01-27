//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base.h"

namespace webss
{
	template <class T>
	class BasicTuple
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;
		using reverse_iterator = typename Data::reverse_iterator;
		using const_reverse_iterator = typename Data::const_iterator;

		explicit BasicTuple(bool containerText = false) : keys(new Keymap()), containerText(containerText) {}
		BasicTuple(Data&& data, bool containerText = false) : keys(nullptr), data(std::move(data)), containerText(containerText) {}

		//instead of the keys being shared, this creates an indepedent copy of the keys and the data
		BasicTuple makeCompleteCopy() const { return BasicTuple(*keys, data, containerText); }

		BasicTuple(const std::shared_ptr<Keymap>& keys, bool containerText = false) : keys(keys), data(keys->size()), containerText(containerText) {}

		explicit operator Data&() { return data; }
		explicit operator const Data&() const { return data; }

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }
		bool isText() const { return containerText; }

		void add(T&& value) { data.push_back(std::move(value)); }
		void add(const T& value) { data.push_back(value); }

		void add(std::string key, T value)
		{
			containerAddUnsafe(*keys, std::move(key), data.size());
			data.push_back(std::move(value));
		}
		void addSafe(std::string key, T value)
		{
			containerAddSafe(*keys, std::move(key), data.size());
			data.push_back(std::move(value));
		}

		bool has(const std::string& key) const { return keys->find(key) != keys->end(); }

		T& back() { return data.back(); }
		const T& back() const { return data.back(); }

		T& operator[](size_type index) { return accessIndexUnsafe<Data, T>(data, index); }
		const T& operator[](size_type index) const { return accessIndexUnsafe<Data, T>(data, index); }
		T& at(size_type index) { return accessIndexSafe<Data, T>(data, index); }
		const T& at(size_type index) const { return accessIndexSafe<Data, T>(data, index); }

		T& operator[](const std::string& key) { return operator[](accessKeyUnsafe<Keymap, size_type>(*keys, key)); }
		const T& operator[](const std::string& key) const { return operator[](accessKeyUnsafe<Keymap, size_type>(*keys, key)); }
		T& at(const std::string& key) { return at(accessKeySafe<Keymap, size_type>(*keys, key)); }
		const T& at(const std::string& key) const { return at(accessKeySafe<Keymap, size_type>(*keys, key)); }

		//returns keys in order of the index they point to
		//they key of indices without an associated key are nullptr
		const std::vector<std::string*> getOrderedKeys() const
		{
			std::vector<std::string*> orderedKeys(size(), nullptr);
			for (const auto& key : *keys)
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

		void merge(const BasicTuple& sender)
		{
			auto orderedKeyValues = sender.getOrderedKeyValues();

			for (const auto& keyValue : orderedKeyValues)
				if (keyValue.first == nullptr)
					add(*keyValue.second);
				else
					addSafe(*keyValue.first, *keyValue.second);
		}

		const std::shared_ptr<Keymap>& getSharedKeys() const { return keys; }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
		reverse_iterator rbegin() { return data.rbegin(); }
		reverse_iterator rend() { return data.rend(); }
		const_reverse_iterator rbegin() const { return data.rbegin(); }
		const_reverse_iterator rend() const { return data.rend(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_TUPLE = "key already in tuple: ";

		std::shared_ptr<Keymap> keys;
		Data data;
		bool containerText;

		//there's a subtle, but important difference with the default copy constructor: default shares the keymap (through the shared pointer),
		//whereas here the keymap itself is moved or copied
		BasicTuple(const Keymap& keymap, const Data& data, bool containerText) : keys(new Keymap(keymap)), data(data), containerText(containerText) {}
	};
}