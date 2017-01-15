//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "utilsSweepers.h"
#include "utilsParser.h"
#include "language.h"
#include "WebssonStructures/entityManager.h"
#include "WebssonUtils/stringBuilder.h"

#define webss_CHAR_ANY_CONTAINER_CHAR_VALUE OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_EQUAL: case CHAR_CSTRING

namespace webss
{
	class FunctionHeadSwitch
	{
	public:
		enum class Type { NONE, STANDARD, BINARY, SCOPED };
		Type t;
		union
		{
			FunctionHeadStandard fheadStandard;
			FunctionHeadBinary fheadBinary;
		};

//		FunctionHeadSwitch() : t(Type::NONE) {}

		FunctionHeadSwitch(FunctionHeadSwitch&& o) { copyUnion(std::move(o)); }

		FunctionHeadSwitch(FunctionHeadStandard&& fhead) : t(Type::STANDARD), fheadStandard(std::move(fhead)) {}
		FunctionHeadSwitch(FunctionHeadBinary&& fhead) : t(Type::BINARY), fheadBinary(std::move(fhead)) {}
/*		FunctionHeadSwitch(const FunctionHeadStandard& fhead) : t(Type::STANDARD), fheadStandard(fhead) {}
		FunctionHeadSwitch(const FunctionHeadBinary& fhead) : t(Type::BINARY), fheadBinary(fhead) {}*/

		FunctionHeadSwitch& operator=(FunctionHeadSwitch&& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(std::move(o));
			}
			return *this;
		}
/*		FunctionHeadSwitch& operator=(const FunctionHeadSwitch& o)
		{
			if (this != &o)
			{
				destroyUnion();
				copyUnion(o);
			}
			return *this;
		}*/

		~FunctionHeadSwitch()
		{
			destroyUnion();
		}

		bool isStandard() { return t == Type::STANDARD; }
		bool isBinary() { return t == Type::BINARY; }
	private:
		void destroyUnion()
		{
			switch (t)
			{
			case Type::STANDARD:
				fheadStandard.~BasicFunctionHead();
				break;
			case Type::BINARY:
				fheadBinary.~BasicFunctionHead();
				break;
			default:
				break;
			}
		}
		void copyUnion(FunctionHeadSwitch&& o)
		{
			switch ((t = o.t))
			{
			case Type::NONE:
				break;
			case Type::STANDARD:
				fheadStandard = std::move(o.fheadStandard);
				break;
			case Type::BINARY:
				fheadBinary = std::move(o.fheadBinary);
				break;
			default:
				throw std::logic_error("");
			}
		}
/*		void copyUnion(const FunctionHeadSwitch& o)
		{
			switch ((t = o.t))
			{
			case Type::NONE:
				break;
			case Type::STANDARD:
				fheadStandard = o.fheadStandard;
				break;
			case Type::BINARY:
				fheadBinary = o.fheadBinary;
				break;
			default:
				throw std::logic_error("");
			}
		}*/
	};

	class Parser
	{
	public:
		using EntityManager = BasicEntityManager<Webss>;
		EntityManager ents;

		Parser();
		Parser(Language lang);

		Document parse(const std::istream& in);
		Document parse(const std::stringstream& in);
		Document parse(const std::string& in);

		void setLanguage(Language lang);
		void addEntity(std::string&& name, Webss&& value);
		void addBlock(std::string&& name, WebssInt value);
	protected:
		Language language;
		char separator;
		bool lineGreed = false;
		bool isVoid;

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

		BasicEntityManager<BlockId> entsBlockId;

		BasicEntityManager<FunctionHeadStandard> entsFheadStandard;
		BasicEntityManager<FunctionHeadBinary> entsFheadBinary;
		BasicEntityManager<WebssBinarySize> entsTypeBinarySize;
		BasicEntityManager<WebssInt> entsTypeInt;

		Document parseDocument(It&& it);
		void parseOption(It& it);
		void checkMultiContainer(It& it, std::function<void()> func);

		Tuple parseTuple(It& it);
		List parseList(It& it);
		Tuple parseTupleText(It& it);
		List parseListText(It& it);
		Dictionary parseDictionary(It& it);
		Namespace parseNamespace(It& it, const std::string& name);
		Enum parseEnum(It& it, const std::string& name);
		Webss parseContainerText(It& it);

		void parseImport(It& it);

		//parserKeyValues.cpp
		Parser::NameType parseNameType(It& it);
		Webss parseCharValue(It& it, ConType con);
		void addJsonKeyvalue(It& it, Dictionary& dict);
		Webss parseValueColon(It& it, ConType con);
		Webss parseValueEqual(It& it, ConType con);
		OtherValue parseOtherValue(It& it, ConType con);
		OtherValue checkOtherValueEntity(It& it, ConType con, const Entity& ent);
		void parseOtherValue(It& it, ConType con, std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Entity& abstractEntity)> funcAbstractEntity);

		//parserNumbers.cpp
		Webss parseNumber(It& it);

		//parserStrings.cpp
		std::string parseLineString(It& it, ConType con);
		std::string parseMultilineString(It& it);
		bool parseMultilineStringLine(It& it, std::string& text, int& countStartEnd);
		std::string parseCString(It& it);
		void checkEscapedChar(It& it, std::string& line);
		bool checkStringEntity(It& it, std::string& line);
		const std::string& parseStringEntity(It& it);

		//parserEntities.cpp
		Entity parseConcreteEntity(It& it);
		Entity parseAbstractEntity(It& it);
		std::string parseName(It& it);
		std::string parseNameSafe(It& it);
		bool nameExists(const std::string& name);
		void parseUsingNamespace(It& it, std::function<void(const Entity& ent)> funcForEach);

		//parserFunctions.cpp
		FunctionHeadSwitch parseFunctionHead(It& it);
		void checkFheadVoid(It& it);
		FunctionHeadStandard parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead);
		FunctionHeadBinary parseFunctionHeadBinary(It & it, FunctionHeadBinary&& fhead);
		BlockHead parseBlockHead(It& it);
		void parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead);
		void parseStandardParameterFunctionHeadText(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadStandardParam(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadStandard(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadText(It& it, FunctionHeadStandard& fhead);
		void parseOtherValuesFheadBinary(It& it, FunctionHeadBinary& fhead);
		FunctionHeadStandard parseFunctionHeadText(It& it);

		Webss parseFunction(It& it);
		Webss parseFunctionText(It& it);
		Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple);
	//	Webss parseFunctionDictionary(It& it, const Tuple& defaultTuple);
		Webss parseFunctionContainer(It& it, const ParamStandard& defaultValue);
		Tuple functionParseTuple(It& it, const FunctionHeadStandard::Tuple& defaultTuple);
		Tuple functionParseTupleText(It& it, const FunctionHeadStandard::Tuple& defaultTuple);
		List functionParseList(It& it, const FunctionHeadStandard::Tuple& defaultTuple, std::function<Tuple(It& it, const FunctionHeadStandard::Tuple& defaultTuple)> func);

		//parserBinary.cpp
		void parseBinaryHead(It& it, FunctionHeadBinary& fhead);
		Tuple parseFunctionBodyBinary(It& it, const FunctionHeadBinary::Tuple& parameters);
		ParamBinary::SizeList parseBinarySizeList(It& it);
		const BasicEntity<WebssBinarySize>& checkEntTypeBinarySize(const Entity& ent);

		//parserUtils.cpp
		bool checkEmptyContainer(It& it, ConType con);
		bool checkNextElementContainer(It & it, ConType con);
		bool checkEmptyContainerVoid(It& it, ConType con, std::function<void()> funcIsVoid);
		bool checkNextElementContainerVoid(It & it, ConType con, std::function<void()> funcIsVoid);




		bool checkSeparator(It& it);
		bool checkSeparatorVoid(It& it, std::function<void()> funcIsVoid);
		void checkContainerEndVoid(It& it, std::function<void()> funcIsVoid);
		
		
		void checkToNextElement(It& it, ConType con);
		bool checkOtherValuesVoid(It& it, std::function<void()> funcIsVoid, std::function<void()> funcIsNameStart, std::function<void()> funcIsNumberStart);
		const BasicEntity<FunctionHeadStandard>& checkEntFheadStandard(const Entity& ent);
		const BasicEntity<FunctionHeadBinary>& checkEntFheadBinary(const Entity& ent);
};
}