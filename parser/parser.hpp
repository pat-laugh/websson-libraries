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
		Webss parseTemplateHead();

		//parserBinary.cpp
		void parseBinaryHead(TemplateHeadBinary& thead);
		Tuple parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params);

		//parserEntities.cpp
		Entity parseConcreteEntity();
		Entity parseAbstractEntity(const Namespace& currentNamespace);

		ParamDocument parseUsingOne();
		ImportedDocument parseImport();

	protected:
		SmartIterator it;
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
		void parseOptionVersion();

		//parserKeyValues.cpp
		Webss parseValueEqual();
		OtherValue parseOtherValueName(std::string&& name);
		OtherValue checkAbstractEntity(const Entity& ent);
		Webss parseValueOnly();

		//parserTemplateHead.cpp
		TemplateHeadStandard parseTemplateHeadStandard(TemplateHeadStandard&& thead = TemplateHeadStandard());
		TemplateHeadBinary parseTemplateHeadBinary(TemplateHeadBinary&& thead = TemplateHeadBinary());
		TemplateHeadStandard parseTemplateHeadText();

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
