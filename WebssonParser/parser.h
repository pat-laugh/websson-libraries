//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "utilsSweepers.h"
#include "language.h"
#include "multiEntityManager.h"
#include "WebssonStructures/webss.h"
#include "WebssonUtils/stringBuilder.h"

#define webss_ALLOW_IMPORT

namespace webss
{
	class Parser
	{
	public:
		MultiEntityManager ents;

		Parser();
		Parser(Language lang);

		Document parse(const std::istream& in);
		Document parse(const std::stringstream& in);
		Document parse(const std::string& in);

		void setLanguage(Language lang);
		void addGlobalEntity(std::string&& name, Webss&& value);
	protected:
		Language language;
		char separator;
		bool lineGreed = false;

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
			enum Type { NAME, KEYWORD, ENTITY };

			NameType(std::string&& name) : type(Type::NAME), name(std::move(name)) {}
			NameType(Keyword keyword) : type(Type::KEYWORD), keyword(keyword) {}
			NameType(const Entity& entity) : type(Type::ENTITY), entity(entity) {}

			Type type;
			std::string name;
			Keyword keyword;
			Entity entity;
		};

		BasicEntityManager<WebssBinarySize> entsTypeBinarySize;
		BasicEntityManager<void*> importedDocuments;

		Document parseDocument(It&& it);

		//returns true if end of container is met, else false
		bool parseDocumentHead(It& it, std::vector<ParamDocument>& docHead, ConType con, const Namespace& nspace);
		void checkMultiContainer(It& it, std::function<void()> func);

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
		Webss parseContainerText(It& it);
		ScopedDocument parseScopedDocument(It& it);
		ImportedDocument parseImport(It& it, ConType con);
		const Namespace& Parser::parseUsingNamespaceStatic(It& it);

		//parserKeyValues.cpp
		Parser::NameType parseNameType(It& it);
		Webss parseCharValue(It& it, ConType con);
		void addJsonKeyvalue(It& it, Dictionary& dict);
		Webss parseValueColon(It& it, ConType con);
		Webss parseValueEqual(It& it, ConType con);
		OtherValue parseOtherValue(It& it, ConType con);
		OtherValue checkOtherValueEntity(It& it, ConType con, const Entity& ent);
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
		std::string parseName(It& it);
		std::string parseNameSafe(It& it);

		//parserFunctions.cpp
		Webss parseFunctionHead(It& it);
		FunctionHeadStandard parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead = FunctionHeadStandard());
		FunctionHeadBinary parseFunctionHeadBinary(It& it, FunctionHeadBinary&& fhead = FunctionHeadBinary());
		FunctionHeadScoped parseFunctionHeadScoped(It& it, FunctionHeadScoped&& fhead = FunctionHeadScoped());
		FunctionHeadText parseFunctionHeadText(It& it, FunctionHeadText&& fhead = FunctionHeadText());
		void parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead);
		void parseStandardParameterFunctionHeadText(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadStandardAfterFhead(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadStandard(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadText(It& it, FunctionHeadText& fhead);
		void parseOtherValuesFheadBinary(It& it, FunctionHeadBinary& fhead);

		Webss parseFunction(It& it, ConType con);
		Webss parseFunctionText(It& it);
		Webss parseFunctionBodyBinary(It& it, const FunctionHeadBinary::Parameters& params);
		Webss parseFunctionBodyScoped(It& it, const FunctionHeadScoped::Parameters& params, ConType con);
		Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Parameters& params);
		Webss parseFunctionBodyText(It& it, const FunctionHeadText::Parameters& params);

		//parserBinary.cpp
		void parseBinaryHead(It& it, FunctionHeadBinary& fhead);
		Tuple parseFunctionTupleBinary(It& it, const FunctionHeadBinary::Parameters& params);
		ParamBinary::SizeList parseBinarySizeList(It& it);
		const BasicEntity<WebssBinarySize>& checkEntTypeBinarySize(const Entity& ent);

		//parserUtils.cpp
		bool checkEmptyContainer(It& it, ConType con);
		bool checkNextElementContainer(It & it, ConType con);
		bool checkEmptyContainerVoid(It& it, ConType con, std::function<void()> funcIsVoid);
		bool checkNextElementContainerVoid(It & it, ConType con, std::function<void()> funcIsVoid);



		

		const BasicEntity<BlockHead>& checkEntBlockHead(const Entity& ent);
		const BasicEntity<FunctionHeadScoped>& checkEntFheadScoped(const Entity& ent);
		const BasicEntity<FunctionHeadBinary>& checkEntFheadBinary(const Entity& ent);
		const BasicEntity<FunctionHeadStandard>& checkEntFheadStandard(const Entity& ent);
		const BasicEntity<FunctionHeadText>& checkEntFheadText(const Entity& ent);
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

		MultiEntityManager& ents;
	public:
		ParamDocumentIncluder(MultiEntityManager& ents, const FunctionHeadScoped::Parameters& params) : ents(ents)
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

				//first check the namespace entity is accessible; if so it has to be removed since it'll no longer be necessary
				//and an entity with the same name could be inside.
				const auto& name = nspace.getName();
				if (ents.hasEntity(name))
				{
					const auto& ent = ents.getWebss(name);
					const auto& content = ent.getContent();
					if (content.isNamespace() && content.getNamespace() == nspace)
						remove(ent);
				}
				for (const auto& ent : nspace)
					include(ent);
				break;
			}
			default:
				assert(false);
			}
		}
	};
}