//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "entityManager.hpp"
#include "utilsSweepers.hpp"
#include "structures/webss.hpp"
#include "structures/documentHead.hpp"
#include "structures/paramBinary.hpp"
#include "structures/tuple.hpp"
#include "structures/list.hpp"
#include "structures/document.hpp"
#include "structures/dictionary.hpp"
#include "structures/paramStandard.hpp"
#include "structures/block.hpp"
#include "structures/template.hpp"
#include "utils/stringBuilder.hpp"
#include "utils/constants.hpp"

namespace webss
{
	class Parser
	{
	public:
		Parser();
		Parser(SmartIterator&& it);
		Parser(const std::istream& in);
		Parser(const std::stringstream& in);
		Parser(const std::string& in);
		Parser& setIterator(SmartIterator&& it);
		Parser& addEntity(std::string&& name, Webss&& value);

		Document parseDocument();

		Tag nextTag;
		ConType con = ConType::DOCUMENT;
		bool multilineContainer = true;
		bool allowVoid = false;

		class ContainerSwitcher;

		template <class Container, ConType::Enum CON>
		Container parseContainer(Container&& cont, bool allowVoid, std::function<void(Container& cont)> func);

		SmartIterator& getIt() { return it; }
		const EntityManager& getEnts() { return ents; }
		ConType getCurrentContainer() { return con; }

		//returns true if container is empty, else false
		bool containerEmpty();

		//returns true if has next element, else false
		bool checkNextElement();

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

		OtherValue parseOtherValue();
		void parseOtherValue(std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);

		std::string parseNameDictionary();

		//parserTemplates.cpp
		Webss parseTemplateHead();
		TemplateHeadStandard parseTemplateHeadText();

		//parserBinary.cpp
		void parseBinaryHead(TemplateHeadBinary& thead);
		Tuple parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params);

		//parserEntities.cpp
		Entity parseConcreteEntity();
		Entity parseAbstractEntity(const Namespace& currentNamespace);

		ParamDocument parseUsingOne();
		ParamDocument parseUsingAll();
		ImportedDocument parseImport();

	protected:
		SmartIterator it;
		EntityManager ents;

		//returns true if end of container is met, else false
		bool parseDocumentHead(std::vector<ParamDocument>& docHead, const Namespace& nspace);

		
		Dictionary parseDictionary(bool isAbstract = false);
		List parseListCommon(bool allowVoid, std::function<void(List&)> defaultFunc);
		List parseList(bool isAbstract = false);
		List parseListText(bool isAbstract = false);
		Tuple parseTupleCommon(bool allowVoid, std::function<void(Tuple&)> defaultFunc);
		Tuple parseTuple(bool isAbstract = false);
		Tuple parseTupleText(bool isAbstract = false);

		Namespace parseNamespace(const std::string& name, const Namespace& previousNamespace);
		Enum parseEnum(const std::string& name);
		void parseOption();
		void parseOptionVersion();

		//parserKeyValues.cpp
		Webss parseValueEqual();
		OtherValue parseOtherValueName(std::string&& name);
		OtherValue checkAbstractEntity(const Entity& ent);
		Webss parseValueOnly();

		Webss parseTemplate();
		Webss parseTemplateText();
		Webss parseTemplateBodyBinary(const TemplateHeadBinary::Parameters& params);
		Webss parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params);
		Webss parseTemplateBodyText(const TemplateHeadStandard::Parameters& params);

		Webss parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue);
		Webss checkTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue, const Webss& value);

		Tuple parseTemplateTupleStandard(const TemplateHeadStandard::Parameters& params);
		Tuple parseTemplateTupleText(const TemplateHeadStandard::Parameters& params);
	};
}
