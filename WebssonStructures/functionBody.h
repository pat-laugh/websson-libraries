//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "tuple.h"
#include "list.h"
#include <cassert>

namespace webss
{
	template <class Webss>
	class BasicFunctionBodySingle
	{
	public:
		using Tuple = BasicTuple<Webss>;

		Tuple tuple;

		BasicFunctionBodySingle(Tuple&& tuple) : tuple(std::move(tuple)) {}
		BasicFunctionBodySingle(const Tuple& tuple) : tuple(tuple) {}

		virtual ~BasicFunctionBodySingle() {}

		BasicFunctionBodySingle(BasicFunctionBodySingle&& o) : tuple(std::move(o.tuple)) {}
		BasicFunctionBodySingle(const BasicFunctionBodySingle& o) : tuple(o.tuple) {}

		BasicFunctionBodySingle& operator=(BasicFunctionBodySingle&& o)
		{
			if (this != &o)
				tuple = std::move(o.tuple);
			return *this;
		}
		BasicFunctionBodySingle& operator=(const BasicFunctionBodySingle& o)
		{
			if (this != &o)
				tuple = o.tuple;
			return *this;
		}

		const Tuple& getTuple() const { return tuple; }

		Webss& operator[](int index) { return tuple[index]; }
		const Webss& operator[](int index) const { return tuple[index]; }
		Webss& at(int index) { return tuple.at(index); }
		const Webss& at(int index) const { return tuple.at(index); }

		Webss& operator[](const std::string& key) { return tuple[key]; }
		const Webss& operator[](const std::string& key) const { return tuple[key]; }
		Webss& at(const std::string& key) { return tuple.at(key); }
		const Webss& at(const std::string& key) const { return tuple.at(key); }
	};

	template <class Webss>
	class BasicFunctionBodyDual
	{
	public:
		using List = BasicList<Webss>;
		using Tuple = BasicTuple<Webss>;

		bool hasList;
		union
		{
			List list;
			Tuple tuple;
		};

		BasicFunctionBodyDual(List&& list) : hasList(true), list(std::move(list)) {}
		BasicFunctionBodyDual(Tuple&& tuple) : hasList(false), tuple(std::move(tuple)) {}
		BasicFunctionBodyDual(const List& list) : hasList(true), list(list) {}
		BasicFunctionBodyDual(const Tuple& tuple) : hasList(false), tuple(tuple) {}

		virtual ~BasicFunctionBodyDual() {}

		BasicFunctionBodyDual(BasicFunctionBodyDual&& o) { copyUnion(std::move(o)); }
		BasicFunctionBodyDual(const BasicFunctionBodyDual& o) { copyUnion(o); }

		BasicFunctionBodyDual& operator=(BasicFunctionBodyDual&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
			return *this;
		}
		BasicFunctionBodyDual& operator=(const BasicFunctionBodyDual& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		const List& getList() const
		{
			assert(hasList);
			return list;
		}
		const Tuple& getTuple() const
		{
			assert(!hasList);
			return tuple;
		}

		Webss& operator[](int index) { return hasList ? list[index] : tuple[index]; }
		const Webss& operator[](int index) const { return hasList ? list[index] : tuple[index]; }
		Webss& at(int index) { return hasList ? list.at(index) : tuple.at(index); }
		const Webss& at(int index) const { return hasList ? list.at(index) : tuple.at(index); }

		Webss& operator[](const std::string& key)
		{
			assert(!hasList);
			return tuple[key];
		}
		const Webss& operator[](const std::string& key) const
		{
			assert(!hasList);
			return tuple[key];
		}
		Webss& at(const std::string& key)
		{
			if (hasList)
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			return tuple.at(key);
		}
		const Webss& at(const std::string& key) const
		{
			if (hasList)
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			return tuple.at(key);
		}
	private:
		static constexpr char* ERROR_LIST_ACCESS_KEY = "can't access list with a key";

		void destroyUnion()
		{
			if (hasList)
				list.~BasicList();
			else
				tuple.~BasicTuple();
		}

		void copyUnion(BasicFunctionBodyDual&& o)
		{
			if ((hasList = o.hasList))
				new (&list) List(std::move(o.list));
			else
				new (&tuple) Tuple(std::move(o.tuple));
		}
		void copyUnion(const BasicFunctionBodyDual& o)
		{
			if ((hasList = o.hasList))
				new (&list) List(o.list);
			else
				new (&tuple) Tuple(o.tuple);
		}
	};
}