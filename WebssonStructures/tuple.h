//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace webss
{
	template <class T>
	class BasicTuple
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using Keymap = std::unordered_map<std::string, size_type>;

		bool containerText;

		explicit BasicTuple(bool containerText = false) : containerText(containerText), keys(new Keymap()) {}
		BasicTuple(Data&& data, bool containerText = false) : containerText(containerText), keys(nullptr), data(std::move(data)) {}
		~BasicTuple() {}

		//instead of the keys being shared, this creates an indepedant copy of the keys and the data
		BasicTuple makeCompleteCopy() const { return BasicTuple(*keys, data, containerText); }

		BasicTuple(const std::shared_ptr<Keymap>& keys, bool containerText = false) : containerText(containerText), keys(keys), data(keys->size()) {}

		BasicTuple(BasicTuple&& o) : containerText(o.containerText), keys(std::move(o.keys)), data(std::move(o.data)) {}
		BasicTuple(const BasicTuple& o) : containerText(o.containerText), keys(o.keys), data(o.data) {}

		BasicTuple& operator=(BasicTuple&& o)
		{
			if (this != &o)
			{
				containerText = o.containerText;
				keys = std::move(o.keys);
				data = std::move(o.data);
			}
			return *this;
		}
		BasicTuple& operator=(const BasicTuple& o)
		{
			if (this != &o)
			{
				containerText = o.containerText;
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
			keys->insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void add(const std::string& key, const T& value)
		{
			keys->insert({ key, data.size() });
			data.push_back(value);
		}

		void addSafe(std::string&& key, T&& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_TUPLE + key);

			keys->insert({ std::move(key), data.size() });
			data.push_back(std::move(value));
		}
		void addSafe(const std::string& key, const T& value)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY_TUPLE + key);

			keys->insert({ key, data.size() });
			data.push_back(value);
		}

		bool has(size_type index) const { index < data.size(); }
		bool has(const std::string& key) const { return keys->find(key) != keys->end(); }

		T& back() { return data.back(); }
		const T& back() const { return data.back(); }

		T& operator[](size_type index) { return data[index]; }
		const T& operator[](size_type index) const { return data[index]; }
		T& at(size_type index) { return data.at(index); }
		const T& at(size_type index) const { return data.at(index); }

		T& operator[](const std::string& key) { return data[keys->find(key)->second]; } //find is better than [] since it does only what it's supposed to do and is as fast
		const T& operator[](const std::string& key) const { return data[keys->find(key)->second]; }
		T& at(const std::string& key) { return data.at(keys->at(key)); }
		const T& at(const std::string& key) const { return data.at(keys->at(key)); }

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

		const Keymap& getCopiedKeys() const { return *keys; }
		const std::shared_ptr<Keymap>& getSharedKeys() const { return keys; }

		typename Data::iterator begin() { return data.begin(); }
		typename Data::iterator end() { return data.end(); }
		typename Data::const_iterator begin() const { return data.begin(); }
		typename Data::const_iterator end() const { return data.end(); }
		typename Data::reverse_iterator rbegin() { return data.rbegin(); }
		typename Data::reverse_iterator rend() { return data.rend(); }
		typename Data::const_reverse_iterator rbegin() const { return data.rbegin(); }
		typename Data::const_reverse_iterator rend() const { return data.rend(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY_TUPLE = "key already in tuple: ";

		std::shared_ptr<Keymap> keys;
		Data data;

		//there's a subtle, but important difference with the default copy constructor: default shares the keymap (through the shared pointer),
		//whereas here the keymap itself is moved or copied
		BasicTuple(const Keymap& keymap, const Data& data, bool containerText) : containerText(containerText), keys(new Keymap(keymap)), data(data) {}
	};
}