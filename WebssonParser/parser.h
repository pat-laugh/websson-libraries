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
		enum class Type { NONE, STANDARD, BINARY, MANDATORY, SCOPE };
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

		Folder parse(const std::string& in);
		Folder parse(std::istream& in);

		void setLanguage(Language lang);
		void addVariable(std::string&& name, Webss&& value);
		void addBlock(std::string&& name, type_int value);
	private:
		Language language;
		char separator;
		bool lineGreed = false;
		bool isVoid;

		BasicVariablesManager<BlockId> varsBlockId;

		BasicVariablesManager<FunctionHeadStandard> varsFunctionHeadStandard;
		BasicVariablesManager<FunctionHeadBinary> varsFunctionHeadBinary;
		BasicVariablesManager<type_binary_size> varsTypeBinarySize;
		BasicVariablesManager<type_int> varsTypeInt;

		Folder parseDocument(It& it);
		void readOption(It& it);
		void getNamespace(It& it);
		void checkMultiContainer(It& it, std::function<void()> func);
		void parseDocumentNameStart(It& it, Document& doc);

		//parserContainers.cpp
		template <const char END, const ConType::Enum CON>
		Tuple parseContainer(It& it)
		{
			Tuple tuple;
			if (checkEmptyContainer(it, CON))
				return tuple;

			do
			{
				switch (*it)
				{
				case END:
					checkContainerEnd(it);
					return tuple;
				case webss_CHAR_ANY_CONTAINER_CHAR_VALUE:
					tuple.add(parseValue(it, CON));
					break;
				default:
					if (checkOtherValues(it, [&]() { parseContainerNameStart(it, tuple, CON); }, [&]() { tuple.add(parseNumber(it)); }))
						continue;
				}
				checkToNextElement(it, CON);
			} while (it);
			throw std::runtime_error(ERROR_CONTAINER_NOT_CLOSED);
		}

		template <const char END, const ConType::Enum CON>
		std::vector<Webss> parseContainerText(It& it)
		{
			std::vector<Webss> data;
			if (checkEmptyContainer(it, CON))
				return data;

			do
			{
				switch (*it)
				{
				case END:
					checkContainerEnd(it);
					return data;
				default:
					if (checkSeparator(it))
						continue;
					data.push_back(parseLineString(it, CON));
					checkToNextElement(it, CON);
				}
			} while (it);
			throw std::runtime_error(ERROR_CONTAINER_NOT_CLOSED);
		}

		Tuple parseTuple(It& it);
		void parseContainerNameStart(It& it, Tuple& tuple, ConType con = ConType::TUPLE);
		List parseList(It& it);
		void parseListNameStart(It& it, List& list);
		Dictionary parseDictionary(It& it);
		Namespace parseNamespaceContainer(It& it, const std::string& name);
		Namespace parseEnum(It& it, const std::string& name);
		Tuple parseTupleText(It& it);
		List parseListText(It& it);
		std::string parseDictionaryText(It& it);
		Block parseBlockValue(It& it, ConType con, const std::string& blockName);
		Block parseBlockValue(It& it, ConType con, const BasicVariable<BlockId>& blockName);

		//parserKeyValues.cpp
		std::pair<std::string, KeyType> parseKey(It& it);
		Webss parseValue(It& it, ConType con);
		void addJsonKeyvalue(It& it, Dictionary& dict);
		Webss parseValueColon(It& it, ConType con);
		Webss parseValueEqual(It& it, ConType con);
		Webss parseValueEqualNameStart(It& it, ConType con);
		const Variable& parseScopedValue(It& it, const std::string& varName);

		//parserNumbers.cpp
		Webss parseNumber(It& it);

		//parserStrings.cpp
		Webss parseContainerString(It& it);
		std::string parseLineString(It& it, ConType con);
		std::string parseLineStringTextDictionary(It& it, int& countStartEnd, bool& addSpace);
		std::string parseCString(It& it);

		//parserVariables.cpp
		Variable parseVariable(It& it);
		Variable parseBlock(It& it);
		std::string parseName(It& it);
		std::string parseNameSafe(It& it);
		std::string parseVariableString(It& it);
		bool nameExists(const std::string& name);

		//parserFunctions.cpp
		FunctionHeadSwitch parseFunctionHead(It& it);
		FunctionHeadSwitch checkFunctionHeadType(It& it, const Variable& var);
		FunctionHeadSwitch checkFunctionHeadType(It& it, const Webss& webss);
		FunctionHeadStandard parseFunctionHeadStandard(It& it, FunctionHeadStandard&& fhead);
		FunctionHeadBinary parseFunctionHeadBinary(It & it, FunctionHeadBinary&& fhead);
		void parseStandardParameterFunctionHead(It& it, FunctionHeadStandard& fhead);
		void parseStandardParameterFunctionHeadText(It& it, FunctionHeadStandard& fhead);
		void parseFunctionHeadBinaryNameStart(It& it, FunctionHeadBinary& fhead);
		FunctionHeadStandard parseFunctionHeadText(It& it);
		void parseFunctionHeadNameStart(It& it, FunctionHeadStandard& fhead);
		void parseFunctionHeadTextNameStart(It& it, FunctionHeadStandard& fhead);

		Webss parseFunction(It& it);
		Webss parseFunctionText(It& it);
		Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple);
	//	Webss parseFunctionDictionary(It& it, const Tuple& defaultTuple);
		Webss parseFunctionContainer(It& it, const ParamStandard& defaultValue);
		Tuple functionParseTuple(It& it, const FunctionHeadStandard::Tuple& defaultTuple);
		void functionParseTupleNameStart(It& it, Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index);
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
		void checkToNextElementVoid(It& it, ConType con);
		bool checkOtherValues(It& it, std::function<void()> funcIsNameStart, std::function<void()> funcIsNumberStart);
		bool checkOtherValuesVoid(It& it, std::function<void()> funcIsVoid, std::function<void()> funcIsNameStart, std::function<void()> funcIsNumberStart);
		void checkEscapedChar(It& it, std::string& line, std::function<void()> funcIsSENT);
		bool checkVariableString(It& it, std::string& line);
		const BasicVariable<FunctionHeadStandard>& checkVariableFunctionHeadStandard(const std::string& name);
		const BasicVariable<FunctionHeadBinary>& checkVariableFunctionHeadBinary(const std::string& name);
		const BasicVariable<type_int>& checkVariableTypeInt(const std::string& name);
	};
}