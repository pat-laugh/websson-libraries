//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "structures/webss.hpp"
#include "structures/list.hpp"
#include "structures/document.hpp"
#include "utils/stringBuilder.hpp"

namespace webss
{
	class SerializerHtml
	{
	public:
		static void serialize(StringBuilder& out, const Document& doc)
		{
			SerializerHtml().putDocument(out, doc);
		}

		static std::string serialize(const Document& doc)
		{
			StringBuilder out;
			serialize(out, doc);
			return out;
		}

	private:
		SerializerHtml();

		void putDocument(StringBuilder& out, const Document& doc);

		void putQuotableValue(StringBuilder& out, const Webss& value);
		void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value);
		void putConcreteValue(StringBuilder& out, const Webss& value);

		void putInt(StringBuilder& out, WebssInt i);
		void putDouble(StringBuilder& out, double d);
		void putQuotableString(StringBuilder& out, const std::string& str);
		void putDocumentString(StringBuilder& out, const std::string& str);
		void putList(StringBuilder& out, const List& list);
		void putTemplate(StringBuilder& out, const Template& templ);
	};
}