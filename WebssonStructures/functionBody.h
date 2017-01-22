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

		This(Dictionary&& dict) : type(WebssType::DICTIONARY), dict(std::move(dict)) {}
		This(List&& list) : type(WebssType::LIST), list(std::move(list)) {}
		This(Tuple&& tuple) : type(WebssType::TUPLE), tuple(std::move(tuple)) {}
		This(const Dictionary& dict) : type(WebssType::DICTIONARY), dict(dict) {}
		This(const List& list) : type(WebssType::LIST), list(list) {}
		This(const Tuple& tuple) : type(WebssType::TUPLE), tuple(tuple) {}

		~This() { destroyUnion(); }

		This(This&& o) { copyUnion(std::move(o)); }
		This(const This& o) { copyUnion(o); }

		This& operator=(This&& o)
		{
			destroyUnion();
			copyUnion(std::move(o));
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
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
			default:
				throw logic_error("");
			}
		}
	private:
		static constexpr char* ERROR_DICTIONARY_ACCESS_INDEX = "can't access dictionary with an index";
		static constexpr char* ERROR_LIST_ACCESS_KEY = "can't access list with a key";

		WebssType type = WebssType::NONE;
		union
		{
			Dictionary dict;
			List list;
			Tuple tuple;
		};

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
			default:
				throw std::logic_error("");
			}
			type = WebssType::NONE;
		}

		void copyUnion(This&& o)
		{
			switch (o.type)
			{
			case WebssType::DICTIONARY:
				new (&dict) Dictionary(std::move(o.dict));
				o.dict.~BasicDictionary();
				break;
			case WebssType::LIST:
				new (&list) List(std::move(o.list));
				o.list.~BasicList();
				break;
			case WebssType::TUPLE:
				new (&tuple) Tuple(std::move(o.tuple));
				o.tuple.~BasicTuple();
				break;
			default:
				throw std::logic_error("");
			}
			type = o.type;
			o.type = WebssType::NONE;
		}
		void copyUnion(const This& o)
		{
			switch (o.type)
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
			default:
				throw std::logic_error("");
			}
			type = o.type;
		}
	};
#undef This
}