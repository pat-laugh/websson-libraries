//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void Deserializer::putFuncBinary(StringBuilder& out, const FunctionBinary& func)
{
	putFheadBinary(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncBinaryDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncBinaryList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncBinaryTuple(out, func.getParameters(), func.getTuple());
		break;
	}
}

void Deserializer::putFuncScoped(StringBuilder& out, const FunctionScoped& func, ConType con)
{
	//include the namespaces
	for (const auto& param : func.getParameters())
		if (param.hasNamespace())
			currentNamespaces.insert(param.getNamespace().getPointer().get());

	if (func.hasEntity())
	{
		putEntityName(out, func.getEntity());
		putKeyValue(out, "", func.getValue(), con);
	}
	else
	{
		putFheadScoped(out, func);
		putWebss(out, func.getValue(), con);
	}

	//remove the namespaces
	for (const auto& param : func.getParameters())
		if (param.hasNamespace())
			currentNamespaces.erase(param.getNamespace().getPointer().get());
}

void Deserializer::putFuncStandard(StringBuilder& out, const FunctionStandard& func)
{
	putFheadStandard(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncStandardDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncStandardList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncStandardTuple(out, func.getParameters(), func.getTuple());
		break;
	}
}

#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
void Deserializer::putParamStandard(StringBuilder& out, FUNC_PARAMS_STANDARD)
{
	if (!value.hasDefaultValue())
	{
		out += key;
		return;
	}
	
	auto webss = value.getDefaultValue();
	switch (webss.t)
	{
	case WebssType::NONE:
		key;
		break;
	default:
		putKeyValue(out, key, webss, ConType::FUNCTION_HEAD);
		break;
	}
}

void Deserializer::putParamText(StringBuilder& out, FUNC_PARAMS_STANDARD)
{
	if (!value.hasDefaultValue())
	{
		out += key;
		return;
	}

	auto webss = value.getDefaultValue();
	switch (webss.t)
	{
	case WebssType::NONE:
		out += key;
		break;
	case WebssType::PRIMITIVE_STRING:
		out += webss.tString->empty() ? key : key + CHAR_COLON + *webss.tString;
		break;
	default:
		throw domain_error("invalid text function parameter type: " + webss.t.toString());
	}
}

void Deserializer::putFheadStandard(StringBuilder& out, const FunctionHeadStandard& fhead)
{
	if (fhead.hasEntity())
	{
		out += OPEN_FUNCTION;
		putEntityName(out, fhead.getEntity());
		out += CLOSE_FUNCTION;
	}
	else if (fhead.empty())
		out += EMPTY_FUNCTION;
	else if (fhead.getParameters().isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putParamsStandard(out, fhead, [&](FUNC_PARAMS_STANDARD) { putParamText(out, key, value); });
	}
	else
		putParamsStandard(out, fhead, [&](FUNC_PARAMS_STANDARD) { putParamStandard(out, key, value); });
}

#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value
void Deserializer::putFheadBinary(StringBuilder& out, const FunctionHeadBinary& fhead)
{
	if (fhead.hasEntity())
	{
		out += OPEN_FUNCTION;
		putEntityName(out, fhead.getEntity());
		out += CLOSE_FUNCTION;
	}
	else if (fhead.empty())
		throw runtime_error("binary function head can't be empty");
	else
		putParamsBinary(out, fhead, [&](FUNC_PARAMS_BINARY) { putParamBinary(out, key, value); });
}

void Deserializer::putParamBinary(StringBuilder& out, FUNC_PARAMS_BINARY)
{
	putBinarySizeHead(out, value.sizeHead);
	out += key;
	putBinarySizeList(out, value.sizeList);
	if (value.sizeHead.getFlag() != ParamBinary::SizeHead::Flag::NONE)
	{
		out += CHAR_EQUAL;
		putValueOnly(out, value.sizeHead.getDefaultValue(), ConType::FUNCTION_HEAD);
	}
}

#undef FUNC_PARAMS_BINARY

void Deserializer::putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& tuple)
{
	out += OPEN_TUPLE;
	putFuncBodyBinary(out, params, tuple);
	out += CLOSE_TUPLE;
}