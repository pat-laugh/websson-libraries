//All rights reserved
//Copyright 2018 Patrick Laughrea
#pragma once

#include "structures/base.hpp"
#include "structures/document.hpp"
#include "various/stringBuilder.hpp"

namespace webss
{
	class SerializerDynamic
	{
	public:
		static void serialize(const Document& doc)
		{
			SerializerDynamic().putDocument(doc);
		}

	private:
		SerializerDynamic();

		void putDocument(const Document& doc);

		void putConcreteValue(const Webss& value);
		
		/*
		void putQuotableValue(various::StringBuilder& out, const Webss& value);
		void putKeyValue(various::StringBuilder& out, const std::string& key, const Webss& value);

		void putInt(various::StringBuilder& out, WebssInt i);
		void putDouble(various::StringBuilder& out, double d);
		void putQuotableString(various::StringBuilder& out, const std::string& str);
		void putDocumentString(various::StringBuilder& out, const std::string& str);
		void putList(various::StringBuilder& out, const List& list);
		void putTemplate(various::StringBuilder& out, const Template& templ);
		void putTemplateFun(various::StringBuilder& out, const Template& templ);
		*/
	};
}