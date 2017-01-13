//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "utilsSweepers.h"
#include "utilsParser.h"
#include "keychars.h"
#include "language.h"
#include "WebssonStructures/variablesManager.h"

#define webss_CHAR_ANY_CONTAINER_CHAR_VALUE OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_EQUAL: case CHAR_CSTRING
#define webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE KeyType::DICTIONARY: case KeyType::LIST: case KeyType::TUPLE: case KeyType::FUNCTION: case KeyType::COLON: case KeyType::EQUAL: case KeyType::CSTRING

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
		using VariablesManager = BasicVariablesManager<Webss>;
		VariablesManager vars;

		Parser();
		Parser(Language lang);

		Document parse(const std::string& in);
		Document parse(std::istream& in);

		void setLanguage(Language lang);
		void addVariable(std::string&& name, Webss&& value);
		void addBlock(std::string&& name, type_int value);
	protected:
		Language language;
		char separator;
		bool lineGreed = false;
		bool isVoid;

		class OtherValue
		{
		public:
			enum class Type { KEY_VALUE, VALUE_ONLY, KEY_ONLY, ABSTRACT_ENTITY, KEY_VALUE_ALIASES, KEY_ONLY_ALIASES };

			OtherValue(std::string&& key, Webss&& value) : type(Type::KEY_VALUE), key(std::move(key)), value(std::move(value)) {}
			OtherValue(Webss&& value) : type(Type::VALUE_ONLY), value(std::move(value)) {}
			OtherValue(std::string&& key) : type(Type::KEY_ONLY), key(std::move(key)) {}
			OtherValue(const Variable& var) : type(Type::ABSTRACT_ENTITY), abstractEntity(var) {}
			OtherValue(std::vector<std::string>&& aliases, Webss&& value) : type(Type::KEY_VALUE_ALIASES), aliases(std::move(aliases)), value(std::move(value)) {}
			OtherValue(std::vector<std::string>&& aliases) : type(Type::KEY_ONLY_ALIASES), aliases(std::move(aliases)) {}

			Type type;
			std::string key;
			Webss value;
			Variable abstractEntity;
			std::vector<std::string> aliases;
		};

		

		BasicVariablesManager<BlockId> varsBlockId;

		BasicVariablesManager<FunctionHeadStandard> varsFheadStandard;
		BasicVariablesManager<FunctionHeadBinary> varsFheadBinary;
		BasicVariablesManager<type_binary_size> varsTypeBinarySize;
		BasicVariablesManager<type_int> varsTypeInt;

		Document parseDocument(It& it);
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

		//parserKeyValues.cpp
		std::pair<std::string, KeyType> parseKey(It& it);
		Webss parseCharValue(It& it, ConType con);
		void addJsonKeyvalue(It& it, Dictionary& dict);
		Webss parseValueColon(It& it, ConType con);
		Webss parseValueEqual(It& it, ConType con);
		const Variable& parseScopedValue(It& it, const std::string& varName);
		OtherValue parseOtherValue(It& it, ConType con);
		OtherValue checkOtherValueVariable(It& it, ConType con, const Variable& var);
		void parseOtherValue(It& it, ConType con, std::function<void(std::string&& key, Webss&& value)> funcKeyValue, std::function<void(std::string&& key)> funcKeyOnly, std::function<void(Webss&& value)> funcValueOnly, std::function<void(const Variable& abstractEntity)> funcAbstractEntity, std::function<void(std::string&& key)> funcAlias);

		//parserNumbers.cpp
		Webss parseNumber(It& it);

		//parserStrings.cpp
		std::string parseLineString(It& it, ConType con);
		std::string parseMultilineString(It& it);
		bool parseMultilineStringLine(It& it, string& text, int& countStartEnd);
		std::string parseCString(It& it);
		void checkEscapedChar(It& it, std::string& line);
		bool checkStringEntity(It& it, std::string& line);
		const std::string& parseStringEntity(It& it);

		//parserVariables.cpp
		void parseConcreteEntity(It& it, std::function<void(const Variable& var)> funcForEach);
		Variable parseAbstractEntity(It& it);
		std::string parseName(It& it);
		std::string parseNameSafe(It& it);
		bool nameExists(const std::string& name);
		void parseUsingNamespace(It& it, std::function<void(const Variable& var)> funcForEach);

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
		ParamBinary::SizeHead parseBinarySizeHead(It& it);
		ParamBinary::SizeList parseBinarySizeList(It& it);
		const BasicVariable<type_binary_size>& checkVariableTypeBinarySize(const std::string& name);

		//parserUtils.cpp
		bool checkSeparator(It& it);
		bool checkSeparatorVoid(It& it, std::function<void()> funcIsVoid);
		void checkContainerEnd(It& it);
		void checkContainerEndVoid(It& it, std::function<void()> funcIsVoid);
		bool checkEmptyContainer(It& it, ConType con);
		bool checkEmptyContainerVoid(It& it, ConType con);
		void checkToNextElement(It& it, ConType con);
		bool checkOtherValuesVoid(It& it, std::function<void()> funcIsVoid, std::function<void()> funcIsNameStart, std::function<void()> funcIsNumberStart);
		const BasicVariable<FunctionHeadStandard>& checkVarFheadStandard(const Variable& var);
		const BasicVariable<FunctionHeadBinary>& checkVarFheadBinary(const Variable& var);
};
}