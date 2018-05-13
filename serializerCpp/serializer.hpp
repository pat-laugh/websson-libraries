//All rights reserved
//Copyright 2018 Patrick Laughrea
#pragma once

#include <string>
#include <unordered_map>

#include "structures/base.hpp"
#include "structures/document.hpp"
#include "various/stringBuilder.hpp"

namespace webss
{
	class SerializerCpp
	{
	public:
		static void serialize(various::StringBuilder& out, const Document& doc)
		{
			SerializerCpp().putDocument(out, doc);
		}

		static std::string serialize(const Document& doc)
		{
			various::StringBuilder out;
			serialize(out, doc);
			return out;
		}

	private:
		std::unordered_map<std::string, std::string> nameSubst;
		
		SerializerCpp();

		void putDocument(various::StringBuilder& out, const Document& doc);
		
		void putConcreteType(various::StringBuilder& out, Webss webss);
		void putEntityName(various::StringBuilder& out, const Entity& ent);
		void putEntityConcrete(various::StringBuilder& out, const Entity& ent);

		void putCommand(various::StringBuilder& out, const Webss& webss);
		void putConcreteValue(various::StringBuilder& out, const Webss& webss);
		void putStringList(various::StringBuilder& out, const StringList& slist);
		void putList(various::StringBuilder& out, const List& list);
		
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