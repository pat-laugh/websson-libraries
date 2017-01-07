//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <vector>

namespace webss
{
	template <class T>
	class BasicList
	{
	public:
		using Data = std::vector<T>;
		using size_type = typename Data::size_type;

		bool containerText;

		explicit BasicList(bool containerText = false) : containerText(containerText) {}
		BasicList(Data&& data, bool containerText = false) : containerText(containerText), data(std::move(data)) {}
		~BasicList() {}

		BasicList(BasicList&& o) : containerText(o.containerText), data(std::move(o.data)) {}
		BasicList(const BasicList& o) : containerText(o.containerText), data(o.data) {}

		BasicList& operator=(BasicList&& o)
		{
			if (this != &o)
			{
				containerText = o.containerText;
				data = std::move(o.data);
			}
			return *this;
		}
		BasicList& operator=(const BasicList& o)
		{
			if (this != &o)
			{
				containerText = o.containerText;
				data = o.data;
			}
			return *this;
		}

		bool empty() const { return data.empty(); }
		size_type size() const { return data.size(); }

		void add(T&& value) { data.push_back(std::move(value)); }
		void add(const T& value) { data.push_back(value); }

		T& operator[](size_type index) { return data[index]; }
		const T& operator[](size_type index) const { return data[index]; }
		T& at(size_type index) { return data.at(index); }
		const T& at(size_type index) const { return data.at(index); }

		typename Data::iterator begin() { return data.begin(); }
		typename Data::iterator end() { return data.end(); }
		typename Data::const_iterator begin() const { return data.begin(); }
		typename Data::const_iterator end() const { return data.end(); }
		typename Data::reverse_iterator rbegin() { return data.rbegin(); }
		typename Data::reverse_iterator rend() { return data.rend(); }
		typename Data::const_reverse_iterator rbegin() const { return data.rbegin(); }
		typename Data::const_reverse_iterator rend() const { return data.rend(); }
	private:
		Data data;
	};
}