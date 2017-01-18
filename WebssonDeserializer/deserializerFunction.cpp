//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

string webss::deserializeFunctionStandard(const FunctionStandard& func)
{
	return deserializeFunctionHeadStandard(func) + (func.isList()
		? deserializeFunctionBodyStandard(func.getParameters(), func.getList())
		: deserializeFunctionBodyStandard(func.getParameters(), func.getTuple()));
}

string webss::deserializeFunctionBinary(const FunctionBinary& func)
{
	return deserializeFunctionHeadBinary(func) + OPEN_TUPLE + deserializeFunctionBodyBinary(func.getParameters(), func.getTuple()) + CLOSE_TUPLE;
}

string webss::deserializeFunctionBodyStandard(const FunctionHeadStandard::Tuple& params, const List& list)
{
	if (list.empty())
		return EMPTY_LIST;

	auto it = list.begin();
	return OPEN_LIST + getSeparatedValues([&]() { return ++it != list.end(); }, [&]() { return deserializeFunctionBodyStandard(params, it->getTuple()); }) + CLOSE_LIST;
}

string webss::deserializeFunctionBodyStandard(const FunctionHeadStandard::Tuple& params, const Tuple& tuple)
{
	if (tuple.empty())
		return EMPTY_TUPLE;

	static const char ERROR_TOO_MANY_ELEMENTS[] = "too many elements in function tuple";
	Tuple::size_type i = 0;
	if (params.containerText)
		return OPEN_TUPLE + getSeparatedValues([&]() { return ++i < tuple.size(); }, [&]()
	{
		if (i == params.size())
			throw runtime_error(ERROR_TOO_MANY_ELEMENTS);
		return deserializeString(tuple[i].getString(), ConType::TUPLE);
	}) + CLOSE_TUPLE;

	return OPEN_TUPLE + getSeparatedValues([&]() { return ++i < tuple.size(); }, [&]()
	{
		if (i == params.size())
			throw runtime_error(ERROR_TOO_MANY_ELEMENTS);
		if (params[i].hasFunctionHead())
		{
			const auto& parameters2 = params[i].getFunctionHeadStandard().getParameters();
			if (tuple[i].isList())
				return deserializeFunctionBodyStandard(parameters2, tuple[i].getList());
			else if (tuple[i].isTuple())
				return deserializeFunctionBodyStandard(parameters2, tuple[i].getTuple());
			else
				throw runtime_error("implementation of function must be a list or tuple");
		}

		const Webss& webss = tuple[i];
		switch (webss.t)
		{
		case WebssType::LIST:
		{
			const auto& list = webss.getList();
			if (!list.empty())
			{
				const auto& webss2 = list[0];
				if (webss2.isTuple() && (&webss2.getTuple().getSharedKeys() == &params.getSharedKeys()))
					return CHAR_SELF + deserializeFunctionBodyStandard(params, list);
			}
			return getValueOnly(webss, ConType::TUPLE);
		}
		case WebssType::TUPLE:
			if (&webss.getTuple().getSharedKeys() == &params.getSharedKeys())
				return CHAR_SELF + deserializeFunctionBodyStandard(params, webss.getTuple());
		default:
			return getValueOnly(webss, ConType::TUPLE);
		}
	}) + CLOSE_TUPLE;
}

#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
string webss::deserializeParametersStandard(const FunctionHeadStandard& fhead, function<string(FUNC_PARAMS_STANDARD)> func)
{
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	auto it = keyValues.begin();
	return OPEN_FUNCTION + getSeparatedValues([&]() { return ++it != keyValues.end(); }, [&]() { if (it->first == nullptr) throw runtime_error(ERROR_ANONYMOUS_KEY); return func(*it->first, *it->second); }) + CLOSE_FUNCTION;
}

string webss::deserializeParameter(FUNC_PARAMS_STANDARD)
{
	if (!value.hasDefaultValue())
		return key;
	
	auto webss = value.getDefaultValue();
	switch (webss.t)
	{
	case WebssType::NONE:
		return key;
	case WebssType::VARIABLE:
		return key + CHAR_EQUAL + webss.ent.getName();
	case WebssType::FUNCTION_STANDARD: case WebssType::FUNCTION_BINARY: case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		return key + CHAR_EQUAL + deserializeWebss(webss);
	case WebssType::PRIMITIVE_STRING:
		return key + CHAR_COLON + deserializeString(*webss.tString, ConType::FUNCTION_HEAD);
	default:
		return key + deserializeWebss(webss);
	}
}

string webss::deserializeParameterString(FUNC_PARAMS_STANDARD)
{
	if (!value.hasDefaultValue())
		return key;

	auto webss = value.getDefaultValue();
	switch (webss.t)
	{
	case WebssType::NONE:
		return key;
	case WebssType::PRIMITIVE_STRING:
		return webss.tString->empty() ? key : key + CHAR_COLON + *webss.tString;
	default:
		throw domain_error("invalid text function parameter type: " + webss.t.toString());
	}
}

string webss::deserializeFunctionHeadStandard(const FunctionHeadStandard& fhead)
{
	if (fhead.hasEntity())
		return OPEN_FUNCTION + fhead.getEntName() + CLOSE_FUNCTION;
	if (fhead.empty())
		return EMPTY_FUNCTION;
	if (fhead.getParameters().containerText)
		return ASSIGN_CONTAINER_STRING + deserializeParametersStandard(fhead, [](FUNC_PARAMS_STANDARD) { return deserializeParameterString(key, value); });
	return deserializeParametersStandard(fhead, [](FUNC_PARAMS_STANDARD) { return deserializeParameter(key, value); });
}

#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value
string webss::deserializeFunctionHeadBinary(const FunctionHeadBinary& fhead)
{
	if (fhead.hasEntity())
		return OPEN_FUNCTION + fhead.getEntName() + CLOSE_FUNCTION;
	if (fhead.empty())
		throw runtime_error("binary function head can't be empty");
	return deserializeParametersBinary(fhead, [](FUNC_PARAMS_BINARY) { return deserializeParameterBinary(key, value); });
}

string webss::deserializeParametersBinary(const FunctionHeadBinary& fhead, function<string(FUNC_PARAMS_BINARY)> func)
{
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	auto it = keyValues.begin();
	return OPEN_FUNCTION + getSeparatedValues([&]() { return ++it != keyValues.end(); }, [&]() { if (it->first == nullptr) throw runtime_error(ERROR_ANONYMOUS_KEY); return func(*it->first, *it->second); }) + CLOSE_FUNCTION;
}

string webss::deserializeParameterBinary(FUNC_PARAMS_BINARY)
{
	string out = deserializeBinarySizeHead(value.sizeHead) + key + deserializeBinarySizeList(value.sizeList);
	if (value.sizeHead.flag != ParamBinary::SizeHead::Flag::NONE)
		out += CHAR_EQUAL + getValueOnly(*value.sizeHead.defaultValue, ConType::FUNCTION_HEAD);
	return out;
}

#undef FUNC_PARAMS_BINARY