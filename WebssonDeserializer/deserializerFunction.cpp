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
#undef FUNC_PARAMS_STANDARD


#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value

void Deserializer::putParamBinary(StringBuilder& out, FUNC_PARAMS_BINARY)
{
	putBinarySizeHead(out, value.sizeHead);
	out += key;
	putBinarySizeList(out, value.sizeList);
	if (value.sizeHead.getFlag() != ParamBinary::SizeHead::Flag::NONE)
	{
		out += CHAR_EQUAL;
		putWebss(out, value.sizeHead.getDefaultValue(), ConType::FUNCTION_HEAD);
	}
}

#undef FUNC_PARAMS_BINARY