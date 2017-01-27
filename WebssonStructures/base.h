#pragma once

#include <vector>
#include <cassert>

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
		assert(it != cont.end() && ("key not in container: " + key).c_str());
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
		assert(cont.find(key) != cont.end() && ("key already in container" + key).c_str());
		cont.insert({ std::move(key), std::move(value) });
	}

	template <class Container, class T>
	void containerAddSafe(Container& cont, std::string&& key, T&& value)
	{
		if (cont.find(key) == cont.end())
			throw std::runtime_error("key already in container" + key + key);
		cont.insert({ std::move(key), std::move(value) });
	}

	template <class Webss>
	class BasicBlockHead;

	template <class Webss>
	class BasicBlock;

	template <class Webss>
	class BasicDictionary;

	template <class Webss>
	class BasicDocument;

	template <class Webss>
	class BasicParamDocument;

//	template <class Webss>
//	using BasicDocumentHead = std::vector<BasicParamDocument<Webss>>;

	template <class Webss>
	class BasicScopedDocument;

	template <class Webss>
	class BasicImportedDocument;

	template <class Webss>
	class BasicParamDocument;

	template <class Webss>
	class BasicNamespace;

	template <class Webss>
	class BasicEntity;

	template <class Parameter>
	class BasicFunctionHead;

	template <class Webss>
	class BasicFunctionBody;

	template <class FunctionHead, class Webss>
	class BasicFunction;

//	template <class Webss>
//	class BasicParamScoped;

//	template <class Webss>
//	class BasicFunctionHeadScoped;

	template <class Webss>
	class BasicFunctionScoped;

	template <class Webss>
	class BasicList;

	template <class Webss>
	class BasicParamBinary;

	template <class Webss>
	class BasicParamStandard;

	template <class Webss>
	class BasicTuple;
}