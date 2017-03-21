//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "entityManager.h"
#include "language.h"
#include "utilsSweepers.h"
#include "WebssonStructures/webss.h"
#include "WebssonUtils/stringBuilder.h"
#include "WebssonUtils/constants.h"

namespace webss
{
	class GlobalParser
	{
	public:
		GlobalParser();
		GlobalParser(SmartIterator&& it);
		GlobalParser(const std::istream& in);
		GlobalParser(const std::stringstream& in);
		GlobalParser(const std::string& in);
		GlobalParser& setIterator(SmartIterator&& it);
		GlobalParser& setLanguage(Language lang);
		GlobalParser& addEntity(std::string&& name, Webss&& value);
		Document parse();
	private:
		BasicEntityManager<Webss> ents;
		BasicEntityManager<void*> importedDocuments;
		SmartIterator it;
		Language language = Language::DEFAULT;
		char separator = CHAR_SEPARATOR;
	public:
		class Parser
		{
		public:
			static Document parseDocument(GlobalParser& globalParser)
			{
				Parser parser(globalParser, ConType::DOCUMENT);
				parser.multiLineContainer = true;
				return parser.parseDocument();
			}
		protected:
			Parser(GlobalParser& globalParser, ConType con)
				: ents(globalParser.ents), importedDocuments(globalParser.importedDocuments), it(globalParser.it)
				, con(con), language(globalParser.language), separator(globalParser.separator) {}

			BasicEntityManager<Webss>& ents;
			BasicEntityManager<void*>& importedDocuments;
			SmartIterator& it;
			ConType con;
			Language language;
			char separator;
			bool lineGreed = false;
			bool multiLineContainer;
			bool empty;
			bool allowVoid;
			Tag nextElem;


			Parser(Parser& parser, ConType con, bool allowVoid);

			//returns true if container is empty, else false
			bool parserContainerEmpty();

			//returns true if has next element, else false
			bool parserCheckNextElement();

			Parser makeImportParser(SmartIterator& newIt)
			{
				return Parser(ents, importedDocuments, newIt);
			}
			Parser(BasicEntityManager<Webss>& ents, BasicEntityManager<void*>& importedDocuments, SmartIterator& it)
				: ents(ents), importedDocuments(importedDocuments), it(it), con(ConType::DOCUMENT), language(Language::DEFAULT), separator(CHAR_SEPARATOR) {}

			Document parseDocument();

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
			bool parseDocumentHead(std::vector<ParamDocument>& docHead, ConType con, const Namespace& nspace);

			template <class Container, ConType::Enum CON>
			Container parseContainer(Container&& cont, std::function<void(Container& cont, ConType con)> func)
			{
				if (!checkEmptyContainer(CON))
					do
						func(cont, CON);
				while (checkNextElementContainer(CON));
				return move(cont);
			}

			Tuple parseTuple();
			List parseList();
			Tuple parseTupleText();
			List parseListText();
			Dictionary parseDictionary();
			Namespace parseNamespace(const std::string& name, const Namespace& previousNamespace);
			Enum parseEnum(const std::string& name);
			void parseScopedDocument(std::vector<ParamDocument>& docHead);
			ImportedDocument parseImport(ConType con);
			const Namespace& GlobalParser::Parser::parseUsingNamespaceStatic();

			//parserKeyValues.cpp
			GlobalParser::Parser::NameType parseNameType();
			Webss parseCharValue(ConType con);
			void addJsonKeyvalue(Dictionary& dict);
			Webss parseValueEqual(ConType con);
			OtherValue parseOtherValue(ConType con);
			OtherValue checkAbstractEntity(ConType con, const Entity& ent);
			void parseOtherValue(ConType con, std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);
			Webss parseValueOnly(ConType con);

			//parserNumbers.cpp
			Webss parseNumber();

			//parserStrings.cpp
			std::string parseLineString(ConType con);
			std::string parseMultilineString();
			std::string parseCString();
			void checkEscapedChar(StringBuilder& line);
			bool checkStringEntity(StringBuilder& line);
			const std::string& parseStringEntity();

			//parserEntities.cpp
			Entity parseConcreteEntity(ConType con);
			Entity parseAbstractEntity(const Namespace& currentNamespace);
			std::string parseNameSafe();

			//parserTemplates.cpp
			Webss parseTemplateHead();
			TemplateHeadStandard parseTemplateHeadText();
			TemplateHeadStandard parseTemplateHeadStandard(TemplateHeadStandard&& thead = TemplateHeadStandard());
			TemplateHeadBinary parseTemplateHeadBinary(TemplateHeadBinary&& thead = TemplateHeadBinary());
			TemplateHeadScoped parseTemplateHeadScoped(TemplateHeadScoped&& thead = TemplateHeadScoped());
			void parseStandardParameterTemplateHead(TemplateHeadStandard& thead);
			void parseOtherValuesTheadStandardAfterThead(TemplateHeadStandard& thead);

			Webss parseTemplate(ConType con);
			Webss parseTemplateText();
			Webss parseTemplateBodyBinary(const TemplateHeadBinary::Parameters& params);
			Webss parseTemplateBodyScoped(const TemplateHeadScoped::Parameters& params, ConType con);
			Webss parseTemplateBodyStandard(const TemplateHeadStandard::Parameters& params);
			Webss parseTemplateBodyText(const TemplateHeadStandard::Parameters& params);

			//parserBinary.cpp
			void parseBinaryHead(TemplateHeadBinary& thead);
			Tuple parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params);
			ParamBinary::SizeList parseBinarySizeList();
			const Entity& checkEntTypeBinarySize(const Entity& ent);

			//parserUtils.cpp
			bool checkEmptyContainer(ConType con);
			bool checkNextElementContainer(ConType con);
			bool checkEmptyContainerVoid(ConType con, std::function<void()> funcIsVoid);
			bool checkNextElementContainerVoid(ConType con, std::function<void()> funcIsVoid);
		};
	};

	

	class ParamDocumentIncluder
	{
	private:
		std::vector<Entity> entitiesToReAdd;
		std::vector<Entity> entitiesToRemove;

		void remove(const Entity& ent)
		{
			entitiesToReAdd.push_back(ent);
			ents.removeLocal(ent);
		}

		void include(const Entity& ent)
		{
			entitiesToRemove.push_back(ent);
			ents.addLocalSafe(ent);
		}

		BasicEntityManager<Webss>& ents;
	public:
		ParamDocumentIncluder(BasicEntityManager<Webss>& ents, const TemplateHeadScoped::Parameters& params) : ents(ents)
		{
			for (const auto& param : params)
				includeEntities(param);
		}

		~ParamDocumentIncluder()
		{
			for (const auto& ent : entitiesToRemove)
				ents.removeLocal(ent);

			for (const auto& ent : entitiesToReAdd)
				ents.addLocal(ent);
		}

		void includeEntities(const ParamDocument& paramDoc)
		{
			using Type = ParamDocument::Type;
			switch (paramDoc.getType())
			{
			case Type::ENTITY_ABSTRACT:
				include(paramDoc.getAbstractEntity());
				break;
			case Type::ENTITY_CONCRETE:
				include(paramDoc.getConcreteEntity());
				break;
			case Type::NAMESPACE:
			{
				const auto& nspace = paramDoc.getNamespace();

				//first check the namespace entity is accessible; if so it has to be removed since
				//it'll no longer be necessary and an entity with the same name could be inside
				if (namespacePresentScope(ents, nspace))
					remove(ents[nspace.getName()]);

				for (const auto& ent : nspace)
					include(ent);
				break;
			}
			default:
				assert(false); throw std::domain_error("");
			}
		}

		static bool namespacePresentScope(const BasicEntityManager<Webss>& ents, const Namespace& nspace)
		{
			const auto& name = nspace.getName();
			if (!ents.hasEntity(name))
				return false;
			
			//make sure they're the exact same entity, not just two different entities with the same name
			const auto& content = ents[name].getContent();
			return content.isNamespace() && content.getNamespaceSafe() == nspace;
		}
	};
}