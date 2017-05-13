//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <cassert>

#include "base.hpp"
#include "dictionary.hpp"
#include "list.hpp"
#include "tuple.hpp"
#include "typeWebss.hpp"
#include "webss.hpp"

namespace webss
{
	class TemplateBody
	{
	public:
		TemplateBody(Dictionary&& dict);
		TemplateBody(List&& list);
		TemplateBody(Tuple&& tuple);
		TemplateBody(Tuple&& tuple, bool);
		TemplateBody(const Dictionary& dict);
		TemplateBody(const List& list);
		TemplateBody(const Tuple& tuple);
		TemplateBody(const Tuple& tuple, bool);

		WebssType getType() const;

		const Webss& getWebss() const;
		const Dictionary& getDictionary() const;
		const List& getList() const;
		const Tuple& getTuple() const;

		bool isDictionary() const;
		bool isList() const;
		bool isTuple() const;
		bool isTupleText() const;

		const Webss& operator[](int index) const;
		const Webss& at(int index) const;

		const Webss& operator[](const std::string& key) const;
		const Webss& at(const std::string& key) const;
	private:
		Webss body;
	};
}