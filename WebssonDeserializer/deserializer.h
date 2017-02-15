//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <memory>
#include <set>

#include "WebssonStructures/webss.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/typeContainers.h"
#include "WebssonUtils/stringBuilder.h"
#include "WebssonUtils/utilsWebss.h"

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
		void putListText(StringBuilder& out, const List& list);
		void putTuple(StringBuilder& out, const Tuple& tuple);
		void putTupleText(StringBuilder& out, const Tuple& tuple);
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

		void putFuncBinary(StringBuilder& out, const TemplateBinary& templ);
		void putFuncScoped(StringBuilder& out, const TemplateScoped& templ, ConType con);
		void putTheadSelf(StringBuilder& out);
		void putFuncStandard(StringBuilder& out, const TemplateStandard& templ);
		void putFuncStandardBody(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Webss& body);

		void putFuncText(StringBuilder& out, const TemplateStandard& templ);

		void putFuncBinaryTuple(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple);
		void putFuncStandardTuple(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple);
		void putFuncStandardTupleText(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple);
		void putFuncTextTuple(StringBuilder& out, const TemplateHeadStandard::Parameters& params, const Tuple& tuple);

		void putTheadBinary(StringBuilder& out, const TemplateHeadBinary& thead);
		void putTheadScoped(StringBuilder & out, const TemplateHeadScoped & thead);
		void putTheadStandard(StringBuilder& out, const TemplateHeadStandard& thead);
		void putTheadText(StringBuilder& out, const TemplateHeadStandard& thead);
		
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
			NamespaceIncluder(std::set<Namespace*>& currentNamespaces, const TemplateHeadScoped::Parameters& params) : currentNamespaces(currentNamespaces)
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