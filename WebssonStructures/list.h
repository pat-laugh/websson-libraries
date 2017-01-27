//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <vector>

#include "base.h"

namespace webss
{
	template <class T>
	class BasicList
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;
		using iterator = typename Data::iterator;
		using const_iterator = typename Data::const_iterator;
		using reverse_iterator = typename Data::reverse_iterator;
		using const_reverse_iterator = typename Data::const_reverse_iterator;

		explicit BasicList(bool containerText = false) : containerText(containerText) {}
		BasicList(Data&& data, bool containerText = false) : data(std::move(data)), containerText(containerText) {}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }
		bool isText() const { return containerText; }

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
	private:
		Data data;
		bool containerText;
	};
}