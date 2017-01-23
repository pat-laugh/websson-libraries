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

void Deserializer::putFuncStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Dictionary& dict)
{
	if (dict.empty())
	{
		 out += EMPTY_DICTIONARY;
		 return;
	}

	auto it = dict.begin();
	out += OPEN_DICTIONARY;
	putSeparatedValues(out, [&]() { return ++it != dict.end(); }, [&]()
	{
		out += it->first;
		if (it->second.isList())
			putFuncStandardList(out, params, it->second.getList());
		else
			putFuncStandardTuple(out, params, it->second.getTuple());
	});
	out += CLOSE_DICTIONARY;
}

void Deserializer::putFuncStandardList(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const List& list)
{
	if (list.empty())
	{
		out += EMPTY_LIST;
		return;
	}

	auto it = list.begin();
	out += OPEN_LIST;
	putSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]() { putFuncStandardTuple(out, params, it->getTuple()); });
	out += CLOSE_LIST;
}

void Deserializer::putFuncStandardTuple(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Tuple& tuple)
{
	if (tuple.empty())
	{
		out += EMPTY_TUPLE;
		return;
	}

	static const char ERROR_TOO_MANY_ELEMENTS[] = "too many elements in function tuple";
	Tuple::size_type i = 0;
	out += OPEN_TUPLE;
	if (params.isText())
	{
		putSeparatedValues(out, [&]() { return ++i < tuple.size(); }, [&]()
		{
			if (i == params.size())
				throw runtime_error(ERROR_TOO_MANY_ELEMENTS);
			putString(out, tuple[i].getString(), ConType::TUPLE);
		});
	}
	else
	{
		putSeparatedValues(out, [&]() { return ++i < tuple.size(); }, [&]()
		{
			if (i == params.size())
				throw runtime_error(ERROR_TOO_MANY_ELEMENTS);
			if (params[i].hasFunctionHead())
			{
				const auto& parameters2 = params[i].getFunctionHeadStandard().getParameters();
				if (tuple[i].isList())
					putFuncStandardList(out, parameters2, tuple[i].getList());
				else if (tuple[i].isTuple())
					putFuncStandardTuple(out, parameters2, tuple[i].getTuple());
				else
					throw runtime_error("implementation of function must be a list or tuple");
				return;
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
					{
						out += CHAR_SELF;
						putFuncStandardList(out, params, list);
					}
				}
				else
					putValueOnly(out, webss, ConType::TUPLE);
				return;
			}
			case WebssType::TUPLE:
				if (&webss.getTuple().getSharedKeys() == &params.getSharedKeys())
				{
					out += CHAR_SELF;
					putFuncStandardTuple(out, params, webss.getTuple());
				}
				return;
			default:
				putValueOnly(out, webss, ConType::TUPLE);
			}
		});
	}
	out += CLOSE_TUPLE;
}

void Deserializer::putFheadScoped(StringBuilder& out, const FunctionHeadScoped& fhead)
{
	if (fhead.hasEntity())
	{
		out += OPEN_FUNCTION;
		putEntityName(out, fhead.getEntity());
		out += CLOSE_FUNCTION;
	}
	else if (fhead.empty())
		out += EMPTY_FUNCTION;
	else
	{
		out += OPEN_FUNCTION;
		auto it = fhead.getParameters().begin();
		putSeparatedValues(out, [&]() { return ++it != fhead.getParameters().end(); }, [&]()
		{
			if (it->hasEntity())
			{
				const auto& ent = it->getEntity();
				const auto& content = ent.getContent();
				out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
				putEntityName(out, ent);
				putKeyValue(out, "", content, ConType::FUNCTION_HEAD);
			}
			else
			{
				out += CHAR_USING_NAMESPACE;
				out += it->getNamespace().getName();
			}
		});
		out += CLOSE_FUNCTION;
	}
}

#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
void Deserializer::putParamsStandard(StringBuilder& out, const FunctionHeadStandard& fhead, function<void(FUNC_PARAMS_STANDARD)> func)
{
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	auto it = keyValues.begin();
	out += OPEN_FUNCTION;
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]()
	{
		if (it->first == nullptr)
			throw runtime_error(ERROR_ANONYMOUS_KEY);
		func(*it->first, *it->second);
	});
	out += CLOSE_FUNCTION;
}

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

void Deserializer::putParamsBinary(StringBuilder& out, const FunctionHeadBinary& fhead, function<void(FUNC_PARAMS_BINARY)> func)
{
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	auto it = keyValues.begin();
	out += OPEN_FUNCTION;
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]()
	{
		if (it->first == nullptr)
			throw runtime_error(ERROR_ANONYMOUS_KEY);
		func(*it->first, *it->second);
	});
	out += CLOSE_FUNCTION;
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

void Deserializer::putFuncBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Dictionary& dict)
{
	if (dict.empty())
	{
		out += EMPTY_DICTIONARY;
		return;
	}

	auto it = dict.begin();
	out += OPEN_DICTIONARY;
	putSeparatedValues(out, [&]() { return ++it != dict.end(); }, [&]()
	{
		out += it->first;
		if (it->second.isList())
			putFuncBinaryList(out, params, it->second.getList());
		else
			putFuncBinaryTuple(out, params, it->second.getTuple());
	});
	out += CLOSE_DICTIONARY;
}

void Deserializer::putFuncBinaryList(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const List& list)
{
	if (list.empty())
	{
		out += EMPTY_LIST;
		return;
	}

	auto it = list.begin();
	out += OPEN_LIST;
	putSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]() { putFuncBinaryTuple(out, params, it->getTuple()); });
	out += CLOSE_LIST;
}

void Deserializer::putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& tuple)
{
	out += OPEN_TUPLE;
	putFuncBodyBinary(out, params, tuple);
	out += CLOSE_TUPLE;
}