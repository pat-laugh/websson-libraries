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
				return parser.parseDocument(parser.it);
			}
		protected:
			BasicEntityManager<Webss>& ents;
			BasicEntityManager<void*>& importedDocuments;
			SmartIterator& it;
			ConType con;
			Language language;
			char separator;
			bool lineGreed = false;
			bool lineContainer;

			Parser(GlobalParser& globalParser, ConType con)
				: ents(globalParser.ents), importedDocuments(globalParser.importedDocuments), it(globalParser.it)
				, language(globalParser.language), separator(globalParser.separator) {}


			using It = SmartIterator;

			Document parseDocument(It& it);

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
			bool parseDocumentHead(It& it, std::vector<ParamDocument>& docHead, ConType con, const Namespace& nspace);

			template <class Container, ConType::Enum CON>
			Container parseContainer(It& it, Container&& cont, std::function<void(Container& cont, ConType con)> func)
			{
				if (!checkEmptyContainer(it, CON))
					do
						func(cont, CON);
				while (checkNextElementContainer(it, CON));
				return move(cont);
			}

			Tuple parseTuple(It& it);
			List parseList(It& it);
			Tuple parseTupleText(It& it);
			List parseListText(It& it);
			Dictionary parseDictionary(It& it);
			Namespace parseNamespace(It& it, const std::string& name, const Namespace& previousNamespace);
			Enum parseEnum(It& it, const std::string& name);
			void parseScopedDocument(It& it, std::vector<ParamDocument>& docHead);
			ImportedDocument parseImport(It& it, ConType con);
			const Namespace& GlobalParser::Parser::parseUsingNamespaceStatic(It& it);

			//parserKeyValues.cpp
			GlobalParser::Parser::NameType parseNameType(It& it);
			Webss parseCharValue(It& it, ConType con);
			void addJsonKeyvalue(It& it, Dictionary& dict);
			Webss parseValueEqual(It& it, ConType con);
			OtherValue parseOtherValue(It& it, ConType con);
			OtherValue checkAbstractEntity(It& it, ConType con, const Entity& ent);
			void parseOtherValue(It& it, ConType con, std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);
			Webss parseValueOnly(It& it, ConType con);

			//parserNumbers.cpp
			Webss parseNumber(It& it);

			//parserStrings.cpp
			std::string parseLineString(It& it, ConType con);
			std::string parseMultilineString(It& it);
			std::string parseCString(It& it);
			void checkEscapedChar(It& it, StringBuilder& line);
			bool checkStringEntity(It& it, StringBuilder& line);
			const std::string& parseStringEntity(It& it);

			//parserEntities.cpp
			Entity parseConcreteEntity(It& it, ConType con);
			Entity parseAbstractEntity(It& it, const Namespace& currentNamespace);
			std::string parseNameSafe(It& it);

			//parserTemplates.cpp
			Webss parseTemplateHead(It& it);
			TemplateHeadStandard parseTemplateHeadText(It& it);
			TemplateHeadStandard parseTemplateHeadStandard(It& it, TemplateHeadStandard&& thead = TemplateHeadStandard());
			TemplateHeadBinary parseTemplateHeadBinary(It& it, TemplateHeadBinary&& thead = TemplateHeadBinary());
			TemplateHeadScoped parseTemplateHeadScoped(It& it, TemplateHeadScoped&& thead = TemplateHeadScoped());
			void parseStandardParameterTemplateHead(It& it, TemplateHeadStandard& thead);
			void parseOtherValuesTheadStandardAfterThead(It& it, TemplateHeadStandard& thead);

			Webss parseTemplate(It& it, ConType con);
			Webss parseTemplateText(It& it);
			Webss parseTemplateBodyBinary(It& it, const TemplateHeadBinary::Parameters& params);
			Webss parseTemplateBodyScoped(It& it, const TemplateHeadScoped::Parameters& params, ConType con);
			Webss parseTemplateBodyStandard(It& it, const TemplateHeadStandard::Parameters& params);
			Webss parseTemplateBodyText(It& it, const TemplateHeadStandard::Parameters& params);

			//parserBinary.cpp
			void parseBinaryHead(It& it, TemplateHeadBinary& thead);
			Tuple parseTemplateTupleBinary(It& it, const TemplateHeadBinary::Parameters& params);
			ParamBinary::SizeList parseBinarySizeList(It& it);
			const Entity& checkEntTypeBinarySize(const Entity& ent);

			//parserUtils.cpp
			bool checkEmptyContainer(It& it, ConType con);
			bool checkNextElementContainer(It & it, ConType con);
			bool checkEmptyContainerVoid(It& it, ConType con, std::function<void()> funcIsVoid);
			bool checkNextElementContainerVoid(It & it, ConType con, std::function<void()> funcIsVoid);
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