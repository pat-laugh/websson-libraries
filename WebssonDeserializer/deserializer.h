//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonUtils/errors.h"
#include "WebssonUtils/endOfLine.h"
#include "WebssonUtils/utilsWebss.h"
#include "WebssonStructures/webss.h"
#include "WebssonStructures/entityManager.h"
#include "WebssonUtils/stringBuilder.h"
#include <functional>

//#define REVERSE_ENDIANNESS_WRITE

namespace webss
{
	using EntityManager = BasicEntityManager<Webss>;
	std::string deserializeAll(const Document& doc);
	std::string deserializeAll(const Document& doc, const EntityManager& ents);

	void deserializeWebss(StringBuilder& out, const Webss& webss);

	void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType stringCon);
	void getValueOnly(StringBuilder& out, const Webss& value, ConType stringCon);

	void getSeparatedValues(StringBuilder& out, std::function<bool()> condition, std::function<void()> output);

	void deserializeString(StringBuilder& out, const std::string& str, ConType con);
	void deserializeDictionary(StringBuilder& out, const Dictionary& dict);
	void deserializeList(StringBuilder& out, const List& list);
	void deserializeTuple(StringBuilder& out, const Tuple& tuple);
	void deserializeDocument(StringBuilder& out, const Document& doc);
	void deserializeBlock(StringBuilder& out, const Block& block);

	void putNamespace(StringBuilder& out, const Namespace& nspace);

	void putEnum(StringBuilder& out, const Enum& tEnum);

	void deserializeFunctionStandard(StringBuilder& out, const FunctionStandard& func);
	void deserializeFunctionBinary(StringBuilder& out, const FunctionBinary& func);

	void deserializeFunctionBodyStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Tuple & params, const Dictionary & dict);
	void deserializeFunctionBodyStandardList(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const List& list);
	void deserializeFunctionBodyStandardTuple(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Tuple& tuple);
#define FUNC_PARAMS_STANDARD StringBuilder& out, const std::string& key, const ParamStandard& value
	void deserializeFunctionHeadStandard(StringBuilder& out, const FunctionHeadStandard& fhead);
	void deserializeParametersStandard(StringBuilder& out, const FunctionHeadStandard& fhead, std::function<std::string(FUNC_PARAMS_STANDARD)> func);
	void deserializeParameter(FUNC_PARAMS_STANDARD);
	void deserializeParameterString(FUNC_PARAMS_STANDARD);
	void deserializeFunctionHeadBinary(StringBuilder& out, const FunctionHeadBinary& fhead);
#undef FUNC_PARAMS_STANDARD

	void deserializeFunctionBodyBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Dictionary& dict);
	void deserializeFunctionBodyBinaryList(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const List& list);
	void deserializeFunctionBodyBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& tuple);

#define FUNC_PARAMS_BINARY StringBuilder& out, const std::string& key, const ParamBinary& value
	void deserializeParameterBinary(FUNC_PARAMS_BINARY);
	void deserializeParametersBinary(StringBuilder& out, const FunctionHeadBinary& fhead, std::function<std::string(FUNC_PARAMS_BINARY)> func);
#undef FUNC_PARAMS_BINARY
	void deserializeFunctionBodyBinary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& data);
	void deserializeBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data);
	void deserializeBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data, std::function<std::string(const Webss& webss)> func);
	void deserializeBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss);
	void deserializeBinarySizeHead(StringBuilder& out, const ParamBinary::SizeHead& bhead);
	void deserializeBinarySizeList(StringBuilder& out, const ParamBinary::SizeList& blist);
}

#ifdef REVERSE_ENDIANNESS_WRITE
#undef REVERSE_ENDIANNESS_WRITE
#endif