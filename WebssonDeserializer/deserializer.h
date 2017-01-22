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

	void putWebss(StringBuilder& out, const Webss& webss, ConType con);

	void putKeyValue(StringBuilder& out, const std::string& key, const Webss& value, ConType con);
	void putValueOnly(StringBuilder& out, const Webss& value, ConType con);

	void putSeparatedValues(StringBuilder& out, std::function<bool()> condition, std::function<void()> output);

	void putLineString(StringBuilder& out, const std::string& str, ConType con);

	void putString(StringBuilder& out, const std::string& str, ConType con);
	void putDictionary(StringBuilder& out, const Dictionary& dict);
	void putList(StringBuilder& out, const List& list);
	void putTuple(StringBuilder& out, const Tuple& tuple);
	void putDocument(StringBuilder& out, const Document& doc);

	void putNamespace(StringBuilder& out, const Namespace& nspace);

	void putEnum(StringBuilder& out, const Enum& tEnum);

	void putBlockHead(StringBuilder& out, const BlockHead& blockHead);
	void putBlock(StringBuilder& out, const Block& block, ConType con);

	void putFuncBinary(StringBuilder& out, const FunctionBinary& func);
	void putFuncScoped(StringBuilder& out, const FunctionScoped& func, ConType con);
	void putFuncStandard(StringBuilder& out, const FunctionStandard& func);

	void putFuncStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Tuple & params, const Dictionary & dict);
	void putFuncStandardList(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const List& list);
	void putFuncStandardTuple(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Tuple& tuple);

	void putFheadBinary(StringBuilder& out, const FunctionHeadBinary& fhead);
	void putFheadScoped(StringBuilder & out, const FunctionHeadScoped & fhead);
	void putFheadStandard(StringBuilder& out, const FunctionHeadStandard& fhead);

#define FUNC_PARAMS_STANDARD const std::string& key, const ParamStandard& value
	void putParamsStandard(StringBuilder& out, const FunctionHeadStandard& fhead, std::function<void(FUNC_PARAMS_STANDARD)> func);
	void putParamStandard(StringBuilder& out, FUNC_PARAMS_STANDARD);
	void putParamText(StringBuilder& out, FUNC_PARAMS_STANDARD);
#undef FUNC_PARAMS_STANDARD

	void putFuncBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Dictionary& dict);
	void putFuncBinaryList(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const List& list);
	void putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& tuple);

#define FUNC_PARAMS_BINARY const std::string& key, const ParamBinary& value
	void putParamBinary(StringBuilder& out, FUNC_PARAMS_BINARY);
	void putParamsBinary(StringBuilder& out, const FunctionHeadBinary& fhead, std::function<void(FUNC_PARAMS_BINARY)> func);
#undef FUNC_PARAMS_BINARY
	void putFuncBodyBinary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& data);
	void putBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data);
	void putBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data, std::function<void(const Webss& webss)> func);
	void putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss);
	void putBinarySizeHead(StringBuilder& out, const ParamBinary::SizeHead& bhead);
	void putBinarySizeList(StringBuilder& out, const ParamBinary::SizeList& blist);
}

#ifdef REVERSE_ENDIANNESS_WRITE
#undef REVERSE_ENDIANNESS_WRITE
#endif