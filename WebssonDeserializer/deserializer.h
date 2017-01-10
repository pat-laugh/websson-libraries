//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonUtils/errors.h"
#include "WebssonUtils/endOfLine.h"
#include "WebssonUtils/utilsWebss.h"
#include "WebssonStructures/webss.h"
#include "WebssonStructures/variablesManager.h"
#include <functional>

//#define REVERSE_ENDIANNESS_WRITE

namespace webss
{
	using VariablesManager = BasicVariablesManager<Webss>;
	std::string deserializeAll(const Document& doc);
	std::string deserializeAll(const Document& doc, const VariablesManager& vars);

	std::string deserializeWebss(const Webss& webss);

	std::string putKeyValue(const std::string& key, const Webss& value, ConType stringCon);
	std::string getValueOnly(const Webss& value, ConType stringCon);

	std::string getSeparatedValues(std::function<bool()> condition, std::function<std::string()> output);

	std::string deserializeString(const std::string& str, ConType con);
	std::string deserializeDictionary(const Dictionary& dictionary);
	std::string deserializeList(const List& list);
	std::string deserializeTuple(const Tuple& tuple);
	std::string deserializeDocument(const Document& doc);
	std::string deserializeBlock(const Block& block);

	std::string putNamespace(const Namespace& nspace);

	std::string putEnum(const Enum& tEnum);

	std::string deserializeFunctionStandard(const FunctionStandard& func);
	std::string deserializeFunctionBinary(const FunctionBinary& func);

	std::string deserializeFunctionBodyStandard(const FunctionHeadStandard::Tuple& parameters, const List& list);
	std::string deserializeFunctionBodyStandard(const FunctionHeadStandard::Tuple& parameters, const Tuple& tuple);
#define FUNC_PARAMS_STANDARD const std::string& key, const ParamStandard& value
	std::string deserializeFunctionHeadStandard(const FunctionHeadStandard& fhead);
	std::string deserializeParametersStandard(const FunctionHeadStandard& fhead, std::function<std::string(FUNC_PARAMS_STANDARD)> func);
	std::string deserializeParameter(FUNC_PARAMS_STANDARD);
	std::string deserializeParameterString(FUNC_PARAMS_STANDARD);
	std::string deserializeFunctionHeadBinary(const FunctionHeadBinary& fhead);
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const std::string& key, const ParamBinary& value
	std::string deserializeParameterBinary(FUNC_PARAMS_BINARY);
	std::string deserializeParametersBinary(const FunctionHeadBinary& fhead, std::function<std::string(FUNC_PARAMS_BINARY)> func);
#undef FUNC_PARAMS_BINARY
	std::string deserializeFunctionBodyBinary(const FunctionHeadBinary::Tuple& parameters, const Tuple& data);
	std::string deserializeBinary(const ParamBinary& bhead, const Webss& data);
	std::string deserializeBinary(const ParamBinary& bhead, const Webss& data, std::function<std::string(const Webss& webss)> func);
	std::string deserializeBinaryElement(const ParamBinary::SizeHead& bhead, const Webss& webss);
	std::string deserializeBinarySizeHead(const ParamBinary::SizeHead& bhead);
	std::string deserializeBinarySizeList(const ParamBinary::SizeList& blist);
}

#ifdef REVERSE_ENDIANNESS_WRITE
#undef REVERSE_ENDIANNESS_WRITE
#endif