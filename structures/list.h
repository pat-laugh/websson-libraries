//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "webss.h"

namespace webss
{
	class List
	{
	public:
		using Data = std::vector<Webss>;
		using size_type = Data::size_type;
		using iterator = Data::iterator;
		using const_iterator = Data::const_iterator;
		using reverse_iterator = Data::reverse_iterator;
		using const_reverse_iterator = Data::const_reverse_iterator;

		List();
		List(Data&& data);
		List(const Data& data);
		~List();

		bool empty() const;
		size_type size() const;

		void add(Webss&& value);
		void add(const Webss& value);

		Webss& operator[](size_type index);
		const Webss& operator[](size_type index) const;
		Webss& at(size_type index);
		const Webss& at(size_type index) const;

		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;
		reverse_iterator rbegin();
		reverse_iterator rend();
		const_reverse_iterator rbegin() const;
		const_reverse_iterator rend() const;
	private:
		Data data;
	};
}