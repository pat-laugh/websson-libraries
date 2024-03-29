//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

#include <functional>
#include <memory>
#include <set>

#include "structures/webss.hpp"
#include "structures/paramBinary.hpp"
#include "structures/tuple.hpp"
#include "structures/list.hpp"
#include "structures/document.hpp"
#include "structures/dictionary.hpp"
#include "structures/paramStandard.hpp"
#include "utils/conType.hpp"
#include "various/stringBuilder.hpp"

namespace webss
{
	class Serializer
	{
	public:
		static void serialize(various::StringBuilder& out, const Document& doc)
		{
			Serializer().putDocument(out, doc);
		}

		static std::string serialize(const Document& doc)
		{
			various::StringBuilder out;
			serialize(out, doc);
			return out;
		}

	protected:
		std::set<void*> currentNamespaces;

		Serializer();

		void putDocument(various::StringBuilder& out, const Document& doc);

		void putPreviousNamespaceNames(various::StringBuilder& out, const Namespace& nspace);
		bool namespaceCurrentScope(const std::weak_ptr<Namespace::NamespaceBody>& nspace);
		void putEntityNameWithoutNamespace(various::StringBuilder & out, const Entity & ent);
		void putEntityName(various::StringBuilder& out, const Entity& ent);

		void putAbstractValue(various::StringBuilder& out, const Webss& webss);

		void putConcreteValue(various::StringBuilder& out, const Webss& webss, ConType con);

		void putCharValue(various::StringBuilder& out, const Webss& value, ConType con);

		void putKeyValue(various::StringBuilder& out, const std::string& key, const Webss& value, ConType con);
		void putExplicitKeyValue(various::StringBuilder& out, const std::string& key, const Webss& value, ConType con);

		void putInt(various::StringBuilder& out, WebssInt i);
		void putDouble(various::StringBuilder& out, double d);
		void putLineString(various::StringBuilder& out, const std::string& str, ConType con);
		void putCString(various::StringBuilder& out, const std::string& str);
		void putStringList(various::StringBuilder& out, const StringList& stringList);
		void putDictionary(various::StringBuilder& out, const Dictionary& dict);
		void putList(various::StringBuilder& out, const List& list);
		void putListText(various::StringBuilder& out, const List& list);
		void putTuple(various::StringBuilder& out, const Tuple& tuple);
		void putTupleText(various::StringBuilder& out, const Tuple& tuple);

		void putThead(various::StringBuilder& out, const Thead& thead, bool decl = false);

		void putImport(various::StringBuilder& out, const ImportedDocument& import, ConType con);

		void putExpandDocumentHead(various::StringBuilder& out, const Namespace& nspace);

		void putNamespace(various::StringBuilder& out, const Namespace& nspace);

		void putEnum(various::StringBuilder& out, const Enum& tEnum);

		void putEntityAbstract(various::StringBuilder& out, const Entity& ent);
		void putEntityConcrete(various::StringBuilder& out, const Entity& ent, ConType con);

		void putTemplate(various::StringBuilder& out, const Template& templ, ConType con);

		void putTemplateBinTuple(various::StringBuilder& out, const Template& templ);
		void putTemplateStdBody(various::StringBuilder & out, const TheadStd::Params & params, const Webss & body);
		void putTemplateStdTuple(various::StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);
		
#ifdef NDEBUG
		void putTemplateStdTupleText(various::StringBuilder& out, const Tuple& tuple);
		void putTemplateTextTuple(various::StringBuilder& out, const Tuple& tuple);
#else
		void putTemplateStdTupleText(various::StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);
		void putTemplateTextTuple(various::StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple);
#endif

		void putBinSizeArray(various::StringBuilder& out, const ParamBin::SizeArray& barray);
		void putParamBin(various::StringBuilder& out, const std::string& key, const ParamBin& param);
		void putParamStd(various::StringBuilder& out, const std::string& key, const ParamStd& param);

		void putTheadBin(various::StringBuilder& out, const TheadBin& thead);

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