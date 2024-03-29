//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "structures/base.hpp"
#include "structures/document.hpp"
#include "various/stringBuilder.hpp"

namespace webss
{
	class SerializerHtml
	{
	public:
		static void serialize(various::StringBuilder& out, const Document& doc)
		{
			SerializerHtml().putDocument(out, doc);
		}

		static std::string serialize(const Document& doc)
		{
			various::StringBuilder out;
			serialize(out, doc);
			return out;
		}

	private:
		SerializerHtml();

		void putDocument(various::StringBuilder& out, const Document& doc);

		void putQuotableValue(various::StringBuilder& out, const Webss& value);
		void putKeyValue(various::StringBuilder& out, const std::string& key, const Webss& value);
		void putConcreteValue(various::StringBuilder& out, const Webss& value);

		void putInt(various::StringBuilder& out, WebssInt i);
		void putDouble(various::StringBuilder& out, double d);
		void putQuotableString(various::StringBuilder& out, const std::string& str);
		void putDocumentString(various::StringBuilder& out, const std::string& str);
		void putList(various::StringBuilder& out, const List& list);
		void putTemplate(various::StringBuilder& out, const Template& templ);
		void putTemplateFun(various::StringBuilder& out, const Template& templ);
	};
}