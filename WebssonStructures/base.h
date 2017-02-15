//MIT License
//Copyright(c) 2016 Patrick Laughrea
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
		assert(cont.find(key) == cont.end() && ("key already in container" + key).c_str());
		cont.insert({ std::move(key), std::move(value) });
	}

	template <class Container, class T>
	void containerAddSafe(Container& cont, std::string&& key, T&& value)
	{
		if (cont.find(key) != cont.end())
			throw std::runtime_error("key already in container" + key + key);
		cont.insert({ std::move(key), std::move(value) });
	}

#define This VectorWrapper
	template <class T>
	class This
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;
		using reverse_iterator = typename Data::reverse_iterator;
		using const_reverse_iterator = typename Data::const_reverse_iterator;

		This() {}
		This(Data&& data) : data(std::move(data)) {}
		This(const Data& data) : data(data) {}
		virtual ~This() {}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(T&& value) { data.push_back(std::move(value)); }
		void add(const T& value) { data.push_back(value); }

		T& operator[](size_type index) { return accessIndexUnsafe<Data, T>(data, index); }
		const T& operator[](size_type index) const { return accessIndexUnsafe<Data, T>(data, index); }
		T& at(size_type index) { return accessIndexSafe<Data, T>(data, index); }
		const T& at(size_type index) const { return accessIndexSafe<Data, T>(data, index); }

		iterator begin() { return data.begin(); }
		iterator end() { return data.end(); }
		const_iterator begin() const { return data.begin(); }
		const_iterator end() const { return data.end(); }
		reverse_iterator rbegin() { return data.rbegin(); }
		reverse_iterator rend() { return data.rend(); }
		const_reverse_iterator rbegin() const { return data.rbegin(); }
		const_reverse_iterator rend() const { return data.rend(); }
	protected:
		Data data;
	};
#undef This

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

	template <class Parameter, class Webss>
	class BasicTemplateHead;

	class TemplateHeadSelf {};

	template <class Webss>
	class BasicTemplateBody;

	template <class TemplateHead, class Webss>
	class BasicTemplate;

	template <class Webss>
	class BasicTemplateScoped;

	template <class Webss>
	class BasicList;

	template <class Webss>
	class BasicParamBinary;

	template <class Webss>
	class BasicParamStandard;

	template <class Webss>
	class BasicTuple;
}