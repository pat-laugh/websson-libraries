//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <functional>
#include <memory>
#include <set>

#include "structures/webss.hpp"
#include "structures/documentHead.hpp"
#include "structures/paramBinary.hpp"
#include "structures/tuple.hpp"
#include "structures/list.hpp"
#include "structures/document.hpp"
#include "structures/dictionary.hpp"
#include "structures/paramStandard.hpp"
#include "utils/conType.hpp"
#include "utils/stringBuilder.hpp"

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

		Serializer();

		void putDocumentHead(StringBuilder& out, const DocumentHead& docHead);
		void putDocument(StringBuilder& out, const Document& doc);

		void putPreviousNamespaceNames(StringBuilder& out, const Namespace& nspace);
		bool namespaceCurrentScope(const std::weak_ptr<Namespace::NamespaceBody>& nspace);
		void putEntityNameWithoutNamespace(StringBuilder & out, const Entity & ent);
		void putEntityName(StringBuilder& out, const Entity& ent);

		void putAbstractValue(StringBuilder& out, const Webss& webss);

		void putConcreteValue(StringBuilder& out, const Webss& webss, ConType con);

		void putCharValue(StringBuilder& out, const Webss& value, ConType con);

		void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType con);
		void putExplicitKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType con);

		void putInt(StringBuilder& out, WebssInt i);
		void putDouble(StringBuilder& out, double d);
		void putLineString(StringBuilder& out, const std::string& str, ConType con);
		void putCString(StringBuilder& out, const std::string& str);
		void putDictionary(StringBuilder& out, const Dictionary& dict);
		void putList(StringBuilder& out, const List& list);
		void putListText(StringBuilder& out, const List& list);
		void putTuple(StringBuilder& out, const Tuple& tuple);
		void putTupleText(StringBuilder& out, const Tuple& tuple);

		void putThead(StringBuilder & out, const Thead& thead);

		void putImport(StringBuilder& out, const ImportedDocument& import);

		void putScopedImport(StringBuilder& out, const Entity& ent, const ImportedDocument& import);
		void putScopedImportList(StringBuilder & out, const std::vector<Entity>& entList, const ImportedDocument & import);

		void putExpandDocumentHead(StringBuilder& out, const Namespace& nspace);

		void putNamespace(StringBuilder& out, const Namespace& nspace);

		void putEnum(StringBuilder& out, const Enum& tEnum);

		void putEntityAbstract(StringBuilder& out, const Entity& ent);
		void putEntityConcrete(StringBuilder& out, const Entity& ent, ConType con);

		void putTemplate(StringBuilder& out, const Template& templ, ConType con);

		void putTemplateBinTuple(StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple);
		void putTemplateStdBody(StringBuilder & out, const TheadStd::Params & params, const Webss & body);
		void putTemplateStdTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);
		void putTemplateStdTupleText(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);
		void putTemplateTextTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);

		void putBinSizeList(StringBuilder& out, const ParamBin::SizeList& blist);
		void putParamBin(StringBuilder& out, const std::string& key, const ParamBin& param);
		void putParamStd(StringBuilder& out, const std::string& key, const ParamStd& param);

		void putTheadBin(StringBuilder& out, const TheadBin& thead);

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
			NamespaceIncluder(std::set<void*>& currentNamespaces, const Namespace& nspace) : currentNamespaces(currentNamespaces)
			{
				include(nspace.getBodyPointerShared().get());
			}

			~NamespaceIncluder()
			{
				for (auto it = toRemove.begin(); it != toRemove.end(); it = toRemove.erase(it))
					currentNamespaces.erase(*it);
			}
		};
	};
}