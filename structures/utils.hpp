//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <cassert>
#include <stdexcept>
#include <string>

namespace webss
{
	template <class Container, class T>
	T& accessIndexUnsafe(const Container& cont, typename Container::size_type index)
	{
		assert(index < cont.size() && "index out of bounds of container");
		return const_cast<T&>(cont[index]);
	}

	template <class Container, class T>
	T& accessIndexSafe(const Container& cont, typename Container::size_type index)
	{
		if (index >= cont.size())
			throw std::runtime_error("index out of bounds of container");
		return const_cast<T&>(cont[index]);
	}

	template <class Container, class T>
	T& accessKeyUnsafe(const Container& cont, const std::string& key)
	{
		auto it = cont.find(key);
		assert(it != cont.end() && "key not in container");
		return const_cast<T&>(it->second);
	}

	template <class Container, class T>
	T& accessKeySafe(const Container& cont, const std::string& key)
	{
		auto it = cont.find(key);
		if (it == cont.end())
			throw std::runtime_error("key not in container: " + key);
		return const_cast<T&>(it->second);
	}

	template <class Container, class T>
	void containerAddUnsafe(Container& cont, std::string&& key, T&& value)
	{
		assert(cont.find(key) == cont.end() && "key already in container");
		cont.insert({ std::move(key), std::move(value) });
	}

	template <class Container, class T>
	void containerAddSafe(Container& cont, std::string&& key, T&& value)
	{
		if (cont.find(key) != cont.end())
			throw std::runtime_error("key already in container: " + key);
		cont.insert({ std::move(key), std::move(value) });
	}

	template <class Container>
	void containerAddSafe(Container& cont, std::string&& key)
	{
		if (cont.find(key) != cont.end())
			throw std::runtime_error("key already in container: " + key);
		cont.insert(std::move(key));
	}

	template <class T>
	bool equalPtrs(const T* ptr1, const T* ptr2)
	{
		return (ptr1 == nullptr && ptr2 == nullptr) || (ptr1 != nullptr && ptr2 != nullptr && *ptr1 == *ptr2);
	}
	template <class T>
	bool equalPtrs(const T& ptr1, const T& ptr2)
	{
		return (ptr1 == nullptr && ptr2 == nullptr) || (ptr1 != nullptr && ptr2 != nullptr && *ptr1 == *ptr2);
	}
}