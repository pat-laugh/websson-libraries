//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <memory>
#include <set>

#include "structures/webss.h"
#include "utils/conType.h"
#include "utils/stringBuilder.h"

namespace webss
{
	class Serializer
	{
	public:
		static void serialize(StringBuilder& out, const Document& doc)
		{
			Serializer().putDocument(out, doc);
		}
		
		static std::string serialize(const Document& doc)
		{
			StringBuilder out;
			serialize(out, doc);
			return out;
		}

		static void serialize(StringBuilder& out, const DocumentHead& docHead)
		{
			Serializer().putDocumentHead(out, docHead);
		}

		static std::string serialize(const DocumentHead& docHead)
		{
			StringBuilder out;
			serialize(out, docHead);
			return out;
		}

	protected:
		std::set<void*> currentNamespaces;

		Serializer() {}


		void putPreviousNamespaceNames(StringBuilder& out, const Namespace& nspace);
		bool namespaceCurrentScope(const Namespace& nspace);
		void putEntityName(StringBuilder& out, const Entity& ent);

		void putAbstractValue(StringBuilder& out, const Webss& webss, ConType con);

		void putConcreteValue(StringBuilder& out, const Webss& webss, ConType con);

		void putCharValue(StringBuilder& out, const Webss& value, ConType con);

		void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType con);

		void putInt(StringBuilder& out, WebssInt i);
		void putDouble(StringBuilder& out, double d);
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
			std::set<void*>& currentNamespaces;
			std::set<void*> toRemove;

			void include(void* ptr)
			{
				currentNamespaces.insert(ptr);
				toRemove.insert(ptr);
			}
		public:
			NamespaceIncluder(std::set<void*>& currentNamespaces, const TemplateHeadScoped::Parameters& params) : currentNamespaces(currentNamespaces)
			{
				for (const auto& param : params)
					if (param.hasNamespace())
						include(param.getNamespace().getPointer().get());
			}

			NamespaceIncluder(std::set<void*>& currentNamespaces, const Namespace& nspace) : currentNamespaces(currentNamespaces)
			{
				include(nspace.getPointer().get());
			}

			NamespaceIncluder(std::set<void*>& currentNamespaces, const Enum& tEnum) : currentNamespaces(currentNamespaces)
			{
				include(tEnum.getPointer().get());
			}

			~NamespaceIncluder()
			{
				for (auto it = toRemove.begin(); it != toRemove.end(); it = toRemove.erase(it))
					currentNamespaces.erase(*it);
			}
		};
	};
}