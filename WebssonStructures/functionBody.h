//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "dictionary.h"
#include "list.h"
#include "tuple.h"
#include "typeWebss.h"
#include <cassert>

namespace webss
{
#define This BasicFunctionBody
	template <class Webss>
	class This
	{
	public:
		using Dictionary = BasicDictionary<Webss>;
		using List = BasicList<Webss>;
		using Tuple = BasicTuple<Webss>;

		WebssType type;
		union
		{
			Dictionary dict;
			List list;
			Tuple tuple;
		};

		This(Dictionary&& dict) : type(WebssType::DICTIONARY), dict(std::move(dict)) {}
		This(List&& list) : type(WebssType::LIST), list(std::move(list)) {}
		This(Tuple&& tuple) : type(WebssType::TUPLE), tuple(std::move(tuple)) {}
		This(const Dictionary& dict) : type(WebssType::DICTIONARY), dict(dict) {}
		This(const List& list) : type(WebssType::LIST), list(list) {}
		This(const Tuple& tuple) : type(WebssType::TUPLE), tuple(tuple) {}

		virtual ~This() {}

		This(This&& o) { copyUnion(std::move(o)); }
		This(const This& o) { copyUnion(o); }

		This& operator=(This&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
			return *this;
		}
		This& operator=(const This& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}

		WebssType getType() const
		{
			return type;
		}

		const Dictionary& getDictionary() const
		{
			assert(type == WebssType::DICTIONARY);
			return dict;
		}
		const List& getList() const
		{
			assert(type == WebssType::LIST);
			return list;
		}
		const Tuple& getTuple() const
		{
			assert(type == WebssType::TUPLE);
			return tuple;
		}

		bool isDictionary() const
		{
			return type == WebssType::DICTIONARY;
		}
		bool isList() const
		{
			return type == WebssType::LIST;
		}
		bool isTuple() const
		{
			return type == WebssType::TUPLE;
		}

		Webss& operator[](int index)
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				throw std::logic_error(ERROR_DICTIONARY_ACCESS_INDEX);
			case WebssType::LIST:
				return list[index];
			case WebssType::TUPLE:
				return tuple[index];
			}
		}
		const Webss& operator[](int index) const
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				throw std::logic_error(ERROR_DICTIONARY_ACCESS_INDEX);
			case WebssType::LIST:
				return list[index];
			case WebssType::TUPLE:
				return tuple[index];
			}
		}
		Webss& at(int index)
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				throw std::logic_error(ERROR_DICTIONARY_ACCESS_INDEX);
			case WebssType::LIST:
				return list.at(index);
			case WebssType::TUPLE:
				return tuple.at(index);
			}
		}
		const Webss& at(int index) const
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				throw std::logic_error(ERROR_DICTIONARY_ACCESS_INDEX);
			case WebssType::LIST:
				return list.at(index);
			case WebssType::TUPLE:
				return tuple.at(index);
			}
		}

		Webss& operator[](const std::string& key)
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				return dict[key];
			case WebssType::LIST:
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			case WebssType::TUPLE:
				return tuple[key];
			}
		}
		const Webss& operator[](const std::string& key) const
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				return dict[key];
			case WebssType::LIST:
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			case WebssType::TUPLE:
				return tuple[key];
			}
		}
		Webss& at(const std::string& key)
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				return dict.at(key);
			case WebssType::LIST:
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			case WebssType::TUPLE:
				return tuple.at(key);
			}
		}
		const Webss& at(const std::string& key) const
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				return dict.at(key);
			case WebssType::LIST:
				throw std::logic_error(ERROR_LIST_ACCESS_KEY);
			case WebssType::TUPLE:
				return tuple.at(key);
			}
		}
	private:
		static constexpr char* ERROR_DICTIONARY_ACCESS_INDEX = "can't access dictionary with an index";
		static constexpr char* ERROR_LIST_ACCESS_KEY = "can't access list with a key";

		void destroyUnion()
		{
			switch (type)
			{
			case WebssType::DICTIONARY:
				dict.~BasicDictionary();
				break;
			case WebssType::LIST:
				list.~BasicList();
				break;
			case WebssType::TUPLE:
				tuple.~BasicTuple();
				break;
			}
		}

		void copyUnion(This&& o)
		{
			switch ((type = o.type))
			{
			case WebssType::DICTIONARY:
				new (&dict) Dictionary(std::move(o.dict));
				break;
			case WebssType::LIST:
				new (&list) List(std::move(o.list));
				break;
			case WebssType::TUPLE:
				new (&tuple) Tuple(std::move(o.tuple));
				break;
			}
		}
		void copyUnion(const This& o)
		{
			switch ((type = o.type))
			{
			case WebssType::DICTIONARY:
				new (&dict) Dictionary(o.dict);
				break;
			case WebssType::LIST:
				new (&list) List(o.list);
				break;
			case WebssType::TUPLE:
				new (&tuple) Tuple(o.tuple);
				break;
			}
		}
	};
#undef This
}