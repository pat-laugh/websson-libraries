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
#include "structures/templateScoped.hpp"
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

		EntityManager getEnts() { return ents; }
	protected:
		EntityManager ents;
		SmartIterator it;
		Tag nextTag;
		ConType con = ConType::DOCUMENT;
		bool multilineContainer = true;
		bool allowVoid = false;

		class ContainerSwitcher;

		template <class Container, ConType::Enum CON>
		Container parseContainer(Container&& cont, std::function<void(Container& cont)> func);

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

		class NameType
		{
		public:
			enum Type { NAME, KEYWORD, ENTITY_ABSTRACT, ENTITY_CONCRETE };

			NameType(std::string&& name) : type(Type::NAME), name(std::move(name)) {}
			NameType(Keyword keyword) : type(Type::KEYWORD), keyword(keyword) {}
			NameType(const Entity& entity) : type(entity.getContent().isAbstract() ? Type::ENTITY_ABSTRACT : Type::ENTITY_CONCRETE), entity(entity) {}

			Type type;
			std::string name;
			Keyword keyword;
			Entity entity;
		};

		//returns true if end of container is met, else false
		bool parseDocumentHead(std::vector<ParamDocument>& docHead, const Namespace& nspace);

		Dictionary parseDictionary();
		std::string parseNameJson();
		void addJsonKeyvalue(Dictionary& dict);
		List parseList();
		Tuple parseTuple();
		List parseListText();
		Tuple parseTupleText();

		Namespace parseNamespace(const std::string& name, const Namespace& previousNamespace);
		Enum parseEnum(const std::string& name);
		TemplateHeadScoped parseScopedDocumentHead();
		DocumentHead parseScopedDocumentBody(const TemplateHeadScoped& head);
		ParamDocument parseScopedDocument();
		ParamDocument parseUsingOne();
		ParamDocument parseUsingAll();
		ImportedDocument parseImport();
		void parseOption();
		void parseOptionVersion();

		//parserKeyValues.cpp
		NameType parseNameType();
		Webss parseValueEqual();
		OtherValue parseOtherValue();
		OtherValue parseOtherValueName(std::string&& name);
		OtherValue checkAbstractEntity(const Entity& ent);
		void parseOtherValue(std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);
		Webss parseValueOnly();

		//parserNumbers.cpp
		Webss parseNumber();

		//parserStrings.cpp
		std::string parseLineString();
		std::string parseMultilineString();
		std::string parseCString();
		void checkEscapedChar(StringBuilder& line);
		bool checkStringEntity(StringBuilder& line);
		const std::string& parseStringEntity();

		//parserEntities.cpp
		Entity parseConcreteEntity();
		Entity parseAbstractEntity(const Namespace& currentNamespace);
		std::string parseNameExplicit();

		//parserTemplates.cpp
		Webss parseTemplateHead();
		TemplateHeadStandard parseTemplateHeadText();

		//only called from parseTemplateHead
		TemplateHeadStandard parseTemplateHeadStandard(TemplateHeadStandard&& thead = TemplateHeadStandard());
		TemplateHeadBinary parseTemplateHeadBinary(TemplateHeadBinary&& thead = TemplateHeadBinary());
		TemplateHeadScoped parseTemplateHeadScoped(TemplateHeadScoped&& thead = TemplateHeadScoped());

		void parseStandardParameterTemplateHead(TemplateHeadStandard& thead);
		void parseOtherValuesTheadStandardAfterThead(TemplateHeadStandard& thead);

		Webss parseTemplate();
		Webss parseTemplateText();
		Webss parseTemplateBodyBinary(const TemplateHeadBinary::Parameters& params);
		Webss parseTemplateBodyScoped(const TemplateHeadScoped::Parameters& params);
		Webss parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params);
		Webss parseTemplateBodyText(const TemplateHeadStandard::Parameters& params);

		Webss parseTemplateContainer(const TemplateHeadStandard::Parameters& params, const ParamStandard& defaultValue);

		Tuple parseTemplateTupleStandard(const TemplateHeadStandard::Parameters& params);
		Tuple parseTemplateTupleText(const TemplateHeadStandard::Parameters& params);

		//parserBinary.cpp
		void parseBinaryHead(TemplateHeadBinary& thead);
		Tuple parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params);
		ParamBinary::SizeList parseBinarySizeList();
		const Entity& checkEntTypeBinarySize(const Entity& ent);
		const Entity& checkEntTypeBinarySizeBits(const Entity& ent);
	};
}
