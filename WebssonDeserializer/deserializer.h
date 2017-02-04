//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonUtils/errors.h"
#include "WebssonUtils/endOfLine.h"
#include "WebssonUtils/utilsWebss.h"
#include "WebssonUtils/stringBuilder.h"
#include "WebssonStructures/webss.h"
#include <functional>
#include <set>
#include <memory>

//#define REVERSE_ENDIANNESS_WRITE

namespace webss
{
	class Deserializer
	{
	public:
		static void deserialize(StringBuilder& out, const Document& doc)
		{
			Deserializer().putDocument(out, doc);
		}
		
		static std::string deserialize(const Document& doc)
		{
			StringBuilder out;
			deserialize(out, doc);
			return out;
		}

		static void deserialize(StringBuilder& out, const DocumentHead& docHead)
		{
			Deserializer().putDocumentHead(out, docHead);
		}

		static std::string deserialize(const DocumentHead& docHead)
		{
			StringBuilder out;
			deserialize(out, docHead);
			return out;
		}

	protected:
		std::set<Namespace*> currentNamespaces;

		Deserializer() {}


		void putNamespaceName(StringBuilder& out, const Namespace& nspace);
		void putEntityName(StringBuilder& out, const Entity& ent);

		void putAbstractValue(StringBuilder& out, const Webss& webss, ConType con);

		void putConcreteValue(StringBuilder& out, const Webss& webss, ConType con);

		void putCharValue(StringBuilder& out, const Webss& value, ConType con);

		void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType con);

		void putLineString(StringBuilder& out, const std::string& str, ConType con);
		void putCstring(StringBuilder& out, const std::string& str);
		void putDictionary(StringBuilder& out, const Dictionary& dict);
		void putList(StringBuilder& out, const List& list);
		void putTuple(StringBuilder& out, const Tuple& tuple);
		void putDocumentHead(StringBuilder& out, const DocumentHead& docHead);
		void putDocument(StringBuilder& out, const Document& doc);

		void putImportedDocument(StringBuilder& out, const ImportedDocument& importDoc, ConType con);

		void putScopedDocument(StringBuilder& out, const ScopedDocument& scopedDoc);

		void putUsingNamespace(StringBuilder& out, const Namespace& nspace);

		void putNamespace(StringBuilder& out, const Namespace& nspace);

		void putEnum(StringBuilder& out, const Enum& tEnum);

		void putBlockHead(StringBuilder& out, const BlockHead& blockHead);
		void putBlock(StringBuilder& out, const Block& block, ConType con);

		void putAbstractEntity(StringBuilder& out, const Entity& ent, ConType con);
		void putConcreteEntity(StringBuilder& out, const Entity& ent, ConType con);

		void putFuncBinary(StringBuilder& out, const FunctionBinary& func);
		void putFuncScoped(StringBuilder& out, const FunctionScoped& func, ConType con);
		void putFheadSelf(StringBuilder& out);
		void putFuncStandard(StringBuilder& out, const FunctionStandard& func);

		void putFuncText(StringBuilder& out, const FunctionText& func);

		void putFuncStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Parameters & params, const Dictionary & dict);
		void putFuncStandardList(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const List& list);
		void putFuncStandardTuple(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const Tuple& tuple);
		void putFuncStandardTupleText(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const Tuple& tuple);

		void putFuncTextDictionary(StringBuilder& out, const FunctionHeadText::Parameters & params, const Dictionary & dict);
		void putFuncTextList(StringBuilder& out, const FunctionHeadText::Parameters& params, const List& list);
		void putFuncTextTuple(StringBuilder& out, const FunctionHeadText::Parameters& params, const Tuple& tuple);

		void putFheadBinary(StringBuilder& out, const FunctionHeadBinary& fhead);
		void putFheadScoped(StringBuilder & out, const FunctionHeadScoped & fhead);
		void putFheadStandard(StringBuilder& out, const FunctionHeadStandard& fhead);
		void putFheadText(StringBuilder& out, const FunctionHeadText& fhead);

		void putFuncBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Dictionary& dict);
		void putFuncBinaryList(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const List& list);
		void putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Tuple& tuple);

		void putFuncBodyBinary(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Tuple& data);
		void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data);
		void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data, std::function<void(const Webss& webss)> func);
		void putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss);
		
		class NamespaceIncluder
		{
		private:
			std::set<Namespace*>& currentNamespaces;
			std::set<Namespace*> toRemove;

			void include(const Namespace& nspace)
			{
				auto ptr = nspace.getPointer().get();
				currentNamespaces.insert(ptr);
				toRemove.insert(ptr);
			}
		public:
			NamespaceIncluder(std::set<Namespace*>& currentNamespaces, const FunctionHeadScoped::Parameters& params) : currentNamespaces(currentNamespaces)
			{
				for (const auto& param : params)
					if (param.hasNamespace())
						include(param.getNamespace());
			}

			NamespaceIncluder(std::set<Namespace*>& currentNamespaces, const Namespace& nspace) : currentNamespaces(currentNamespaces)
			{
				include(nspace);
			}

			~NamespaceIncluder()
			{
				for (auto it = toRemove.begin(); it != toRemove.end(); it = toRemove.erase(it))
					currentNamespaces.erase(*it);
			}
		};
	};
}

#ifdef REVERSE_ENDIANNESS_WRITE
#undef REVERSE_ENDIANNESS_WRITE
#endif