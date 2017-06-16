//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "entityManager.hpp"
#include "tagIterator.hpp"
#include "utilsSweepers.hpp"
#include "structures/webss.hpp"
#include "structures/documentHead.hpp"
#include "structures/paramBinary.hpp"
#include "structures/tuple.hpp"
#include "structures/list.hpp"
#include "structures/document.hpp"
#include "structures/dictionary.hpp"
#include "structures/paramStandard.hpp"
#include "structures/thead.hpp"
#include "structures/templatePlus.hpp"
#include "utils/stringBuilder.hpp"
#include "utils/constants.hpp"

namespace webss
{
	class Parser
	{
	public:
		Parser(SmartIterator&& it);
		Parser(const std::istream& in);
		Parser(const std::stringstream& in);
		Parser(const std::string& in);
		Parser& setIterator(SmartIterator&& it);
		Parser& addEntity(std::string&& name, Webss&& value);

		Document parseDocument();

		TagIterator tagit;
		ConType con = ConType::DOCUMENT;
		bool multilineContainer = true;
		bool allowVoid = false;
		std::map<std::string, std::vector<std::string>> aliases;

		class ContainerSwitcher;

		template <class Container, ConType::Enum CON>
		Container parseContainer(Container&& cont, bool allowVoid, std::function<void(Container& cont)> func);

		SmartIterator& getIt() { return tagit.getIt(); }
		SmartIterator& getItSafe() { return tagit.getItSafe(); }
		const EntityManager& getEnts() { return ents; }
		ConType getCurrentContainer() { return con; }

		bool hasAlias(const std::string& name);

		//returns true if container is empty, else false
		bool containerEmpty();

		class OtherValue
		{
		public:
			enum Type { KEY_VALUE, VALUE_ONLY, KEY_ONLY, ABSTRACT_ENTITY };

			OtherValue(std::string&& key, Webss&& value) : type(Type::KEY_VALUE), key(std::move(key)), value(std::move(value)) {}
			OtherValue(Webss&& value) : type(Type::VALUE_ONLY), value(std::move(value)) {}
			OtherValue(std::string&& key) : type(Type::KEY_ONLY), key(std::move(key)) {}
			OtherValue(const Entity& ent) : type(Type::ABSTRACT_ENTITY), abstractEntity(ent) {}

			Type type;
			std::string key;
			Webss value;
			Entity abstractEntity;
		};

		OtherValue parseOtherValue(bool explicitName = false);
		void parseOtherValue(std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);
		void parseExplicitKeyValue(std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly);

		//parserTemplates.cpp
		TheadOptions parseTheadOptions();
		Thead parseThead(bool allowSelf = false);

		//parserBin.cpp
		void parseBinHead(TheadBin& thead);
		Tuple parseTemplateTupleBin(const TheadBin::Params& params);

		//parserEntities.cpp
		Entity parseConcreteEntity();
		Entity parseAbstractEntity(const Namespace& currentNamespace);

		ParamDocument parseScopedImport();
		ImportedDocument parseImport();

	protected:
		EntityManager ents;

		//returns true if has next element, else false
		bool checkNextElement();

		//returns true if end of container is met, else false
		bool parseDocumentHead(Document& doc, const Namespace& nspace);

		Dictionary parseDictionary();
		List parseListCommon(std::function<void(List&)> defaultFunc);
		List parseList();
		List parseListText();
		Tuple parseTupleCommon(std::function<void(Tuple&)> defaultFunc);
		Tuple parseTuple();
		Tuple parseTupleText();

		Namespace parseNamespace(const std::string& name, const Namespace& previousNamespace);
		Enum parseEnum(const std::string& name);
		void parseOption();
		void parseOptionVersion(const std::vector<std::string>& items, std::vector<std::string>::size_type& index);
		void parseOptionAlias(const std::vector<std::string>& items, std::vector<std::string>::size_type& index);

		//parserEntities.cpp
		Webss parseAbstractCharValue(const std::string& name, const Namespace& currentNamespace);
		Webss parseAbstractValueEqual(const std::string& name, const Namespace& currentNamespace);
		Webss parseAbstractValueOnly(const std::string& name, const Namespace& currentNamespace);

		//parserKeyValues.cpp
		Webss parseValueEqual();
		Webss parseCharValue();
		OtherValue parseOtherValueName(std::string&& name);
		OtherValue checkAbstractEntity(const Entity& ent);
		Webss parseValueOnly();


		//parserThead.cpp
		TheadStd parseTheadStd(TheadStd&& thead = TheadStd());
		TheadBin parseTheadBin(TheadBin&& thead = TheadBin());
		//TheadStd parseTheadText();

		//Thead parseTheadText();

//		Webss parseTheadPlus();
//		TheadStd parseTheadPlusText();

		Webss parseTemplateContainer(const TheadStd::Params& params, const ParamStd& defaultValue);
		Webss checkTemplateContainer(const TheadStd::Params& params, const ParamStd& defaultValue, const Webss& value);

		Webss buildTemplateBodyStd(const TheadStd::Params& params, const Webss& defaultValue);
		Tuple parseTemplateTupleStd(const TheadStd::Params& params);
		Tuple parseTemplateTupleText(const TheadStd::Params& params);
		Tuple::size_type expandTemplateTuple(const TheadStd::Params& params, Tuple& templateTuple, Tuple::size_type index);
		List buildTemplateBodyList(const TheadStd::Params& params, const List& baseList);
		void fillTemplateBodyList(const TheadStd::Params& params, const List& baseList, List& filledList);
		Tuple buildTemplateBodyTuple(const TheadStd::Params& params, const Tuple& baseTuple);
		Tuple::size_type fillTemplateBodyTuple(const TheadStd::Params& params, const Tuple& baseTuple, Tuple& filledTuple, Tuple::size_type index = 0);

		Webss parseTemplate();
//		Webss parseTemplateText();
		Webss parseTemplateBin(TheadBin thead);
		Webss parseTemplateStd(TheadStd thead);
		Webss parseTemplateText(TheadStd thead);
		Webss parseTemplatePlusBin(TheadBin thead);
		Webss parseTemplatePlusStd(TheadStd thead);
		Webss parseTemplatePlusText(TheadStd thead);
	};
}
