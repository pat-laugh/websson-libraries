//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <cassert>

#include "base.h"
#include "dictionary.h"
#include "list.h"
#include "tuple.h"
#include "typeWebss.h"

namespace webss
{
#define This BasicTemplateBody
	template <class Webss>
	class This
	{
	public:
		using Dictionary = BasicDictionary<Webss>;
		using List = BasicList<Webss>;
		using Tuple = BasicTuple<Webss>;

		This(Dictionary&& dict) : body(std::move(dict)) {}
		This(List&& list) : body(std::move(list)) {}
		This(Tuple&& tuple) : body(std::move(tuple)) {}
		This(Tuple&& tuple, bool) : body(std::move(tuple), true) {}
		This(const Dictionary& dict) : body(dict) {}
		This(const List& list) : body(list) {}
		This(const Tuple& tuple) : body(tuple) {}
		This(const Tuple& tuple, bool) : body(tuple, true) {}

		WebssType getType() const { return body.getTypeRaw(); }

		const Webss& getWebss() const { return body; }
		const Dictionary& getDictionary() const { return body.getDictionaryRaw(); }
		const List& getList() const { return body.getListRaw(); }
		const Tuple& getTuple() const { return body.getTupleRaw(); }

		bool isDictionary() const { return body.isDictionary(); }
		bool isList() const { return body.isList(); }
		bool isTuple() const { return body.isTuple(); }
		bool isTupleText() const { return body.isTupleText(); }

		const Webss& operator[](int index) const { return body[index]; }
		const Webss& at(int index) const { return body.at(index); }

		const Webss& operator[](const std::string& key) const { return body[key]; }
		const Webss& at(const std::string& key) const { return body.at(key); }
	private:
		Webss body;
	};
#undef This
}