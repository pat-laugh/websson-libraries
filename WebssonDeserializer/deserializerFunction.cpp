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

void Deserializer::putFuncText(StringBuilder& out, const FunctionText& func)
{
	putFheadText(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncTextDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncTextList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncTextTuple(out, func.getParameters(), func.getTuple());
		break;
	}
}

#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
void Deserializer::putParamStandard(StringBuilder& out, FUNC_PARAMS_STANDARD)
{
	if (!value.hasDefaultValue())
		out += key;
	else
		putKeyValue(out, key, value.getDefaultValue(), ConType::FUNCTION_HEAD);
}
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_TEXT const string& key, const ParamText& value
void Deserializer::putParamText(StringBuilder& out, FUNC_PARAMS_TEXT)
{
	if (!value.hasDefaultValue())
		out += key;
	else
	{
		const auto& webss = value.getDefaultValue();
		assert(webss.getType() == WebssType::PRIMITIVE_STRING && "function head text paramaters should only be of type string");
		putKeyValue(out, key, webss, ConType::FUNCTION_HEAD);
	}
}
#undef FUNC_PARAMS_TEXT


#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value

void Deserializer::putParamBinary(StringBuilder& out, FUNC_PARAMS_BINARY)
{
	putBinarySizeHead(out, value.sizeHead);
	out += key;
	putBinarySizeList(out, value.sizeList);
	if (value.sizeHead.hasDefaultValue())
		putCharValue(out, value.sizeHead.getDefaultValue(), ConType::FUNCTION_HEAD);
}

#undef FUNC_PARAMS_BINARY