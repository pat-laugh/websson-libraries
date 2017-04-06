//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "entityManager.h"
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
		GlobalParser& addEntity(std::string&& name, Webss&& value);
		Document parse();
	private:
		BasicEntityManager<Webss> ents;
		BasicEntityManager<void*> importedDocuments;
		SmartIterator it;
	public:
		class Parser
		{
		public:
			static Document parseDocument(GlobalParser& globalParser)
			{
				Parser parser(globalParser, ConType::DOCUMENT);
				parser.multilineContainer = true;
				return parser.parseDocument();
			}
		public:
			Parser(GlobalParser& globalParser, ConType con)
				: ents(globalParser.ents), importedDocuments(globalParser.importedDocuments), it(globalParser.it), con(con) {}

			BasicEntityManager<Webss>& ents;
			BasicEntityManager<void*>& importedDocuments;
			SmartIterator& it;
			Tag nextTag;
			ConType con;
			bool multilineContainer;
			bool allowVoid = false;
			bool lineGreed = false;

			class ContainerSwitcher
			{
			private:
				Parser& parser;
				ConType oldCon;
				bool oldAllowVoid;
				bool oldMultilineContainer;
			public:
				ContainerSwitcher(Parser& parser, ConType newCon, bool newAllowVoid) : parser(parser), oldCon(parser.con), oldAllowVoid(parser.allowVoid), oldMultilineContainer(parser.multilineContainer)
				{
					parser.con = newCon;
					parser.allowVoid = newAllowVoid;
					parser.multilineContainer = checkLineEmpty(++parser.it);
				}

				~ContainerSwitcher()
				{
					parser.con = oldCon;
					parser.allowVoid = oldAllowVoid;
					parser.multilineContainer = oldMultilineContainer;
				}
			};

			class ImportSwitcher
			{
			private:
				Parser& parser;
				SmartIterator oldIt;
				Tag oldNextTag;
				ConType oldCon;
				bool oldAllowVoid;
				bool oldMultilineContainer;
				bool oldLineGreed;
			public:
				ImportSwitcher(Parser& parser, SmartIterator&& newIt) : parser(parser), oldIt(std::move(parser.it)), oldNextTag(parser.nextTag), oldCon(parser.con), oldAllowVoid(parser.allowVoid), oldMultilineContainer(parser.multilineContainer), oldLineGreed(parser.lineGreed)
				{
					parser.it = std::move(newIt);
					parser.con = ConType::DOCUMENT;
					parser.allowVoid = false;
					parser.multilineContainer = true;
				}

				~ImportSwitcher()
				{
					parser.it = std::move(oldIt);
					parser.nextTag = oldNextTag;
					parser.con = oldCon;
					parser.allowVoid = oldAllowVoid;
					parser.multilineContainer = oldMultilineContainer;
					parser.lineGreed = oldLineGreed;
				}
			};

			//returns true if container is empty, else false
			bool parserContainerEmpty();

			//returns true if has next element, else false
			bool parserCheckNextElement();

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
			bool parseDocumentHead(std::vector<ParamDocument>& docHead, const Namespace& nspace);

			template <class Container, ConType::Enum CON>
			Container parseContainer(Container&& cont, std::function<void(Container& cont)> func)
			{
				ContainerSwitcher switcher(*this, CON, false);
				if (!parserContainerEmpty())
					do
						func(cont);
					while (parserCheckNextElement());
				return move(cont);
			}

			Dictionary parseDictionary();
			List parseList();
			Tuple parseTuple();
			List parseListText();
			Tuple parseTupleText();
			Namespace parseNamespace(const std::string& name, const Namespace& previousNamespace);
			Enum parseEnum(const std::string& name);
			void parseScopedDocument(std::vector<ParamDocument>& docHead);
			ImportedDocument parseImport();
			const Namespace& GlobalParser::Parser::parseUsingNamespaceStatic();

			//parserKeyValues.cpp
			GlobalParser::Parser::NameType parseNameType();
			Webss parseCharValue();
			void addJsonKeyvalue(Dictionary& dict);
			Webss parseValueEqual();
			OtherValue parseOtherValue();
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
			std::string parseNameSafe();

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

			Webss parseTemplateContainer(const TemplateHeadStandard::Parameters & params, const ParamStandard & defaultValue);

			//parserBinary.cpp
			void parseBinaryHead(TemplateHeadBinary& thead);
			Tuple parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params);
			ParamBinary::SizeList parseBinarySizeList();
			const Entity& checkEntTypeBinarySize(const Entity& ent);
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