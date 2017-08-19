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
#include "structures/template.hpp"
#include "utils/constants.hpp"

namespace webss
{
	class Parser
	{
	public:
		Parser(std::string filename = "");
		Parser(various::SmartIterator it, std::string filename = "");
		Parser(const std::istream& in, std::string filename = "");
		Parser& setIterator(various::SmartIterator it);
		Parser& setFilename(std::string filename);
		Parser& addEntity(std::string name, Webss value);

		Document parseDocument();
		Document parseDocument(various::SmartIterator it, std::string filename = "");
		Document parseDocument(const std::istream& in, std::string filename = "");

		TagIterator tagit;
		ConType con = ConType::DOCUMENT;
		bool multilineContainer = true;
		bool allowVoid = false;
		std::string filename;

		class ContainerSwitcher;

		template <class Container, ConType::Enum CON>
		Container parseContainer(Container&& cont, bool allowVoid, std::function<void(Container& cont)> func);

		various::SmartIterator& getIt() { return tagit.getIt(); }
		various::SmartIterator& getItSafe() { return tagit.getItSafe(); }
		const EntityManager& getEnts() { return ents; }
		ConType getCurrentContainer() { return con; }

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

		//parserEntities.cpp
		Entity parseConcreteEntity();
		Entity parseAbstractEntity(const Namespace& currentNamespace);

#ifndef WEBSSON_PARSER_DISABLE_IMPORT
		ImportedDocument parseImport();
#endif

	protected:
		EntityManager ents;

		//returns true if has next element, else false
		bool checkNextElement();

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

		class ParserThead;
		class ParserTempl;

		Thead parseThead(bool allowSelf = false);
		Webss parseTemplate();
		Webss parseTemplateBody(Thead thead);

		//parserBinary.cpp
		void parseBinHead(TheadBin& thead);
		Tuple parseTemplateTupleBin(const TheadBin::Params& params);
	};
}
