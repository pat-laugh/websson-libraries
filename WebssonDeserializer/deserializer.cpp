//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"
#include <type_traits>

using namespace std;
using namespace webss;

void putContainerStart(StringBuilder& out, ConType con);
void putContainerEnd(StringBuilder& out, ConType con);

class DeserializerTemplate : public Deserializer
{
public:
	template <class T>
	void putEntityName(StringBuilder& out, const BasicEntity<T>& ent)
	{
		if (ent.hasNamespace())
		{
			const auto& entName = ent.getName();
			const auto& entNspace = ent.getNamespace();
			const auto& nspaces = entNspace.getNamespaces();
			decltype(nspaces.size()) i = 0;
			for (; i < nspaces.size(); ++i)
				if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
					break;

			for (; i < nspaces.size(); ++i)
				out += nspaces[i]->getName() + CHAR_SCOPE;
		}

		out += ent.getName();
	}

	template <class T>
	void putFheadEntity(StringBuilder& out, const BasicEntity<T>& ent)
	{
		static const ConType CON = ConType::FUNCTION_HEAD;
		putContainerStart(out, CON);
		putEntityName<T>(out, ent);
		putContainerEnd(out, CON);
	}
};

#define putEntityName static_cast<DeserializerTemplate*>(this)->putEntityName
#define putFheadEntity static_cast<DeserializerTemplate*>(this)->putFheadEntity

template <class T, ConType::Enum CON>
void putSeparatedValues(StringBuilder& out, const T& t, function<void(typename T::const_iterator it)> output)
{
	putContainerStart(out, CON);
	if (!t.empty())
	{
		typename T::const_iterator it = t.begin();
		output(it);
		while (++it != t.end())
		{
			out += CHAR_SEPARATOR;
			output(it);
		}
	}
	putContainerEnd(out, CON);
}

void addCharEscape(StringBuilder& out, char c)
{
	out += '\\';
	switch (c)
	{
	case '\0': out += '0'; break;
	case '\a': out += 'a'; break;
	case '\b': out += 'b'; break;
	case '\f': out += 'f'; break;
	case '\n': out += 'n'; break;
	case '\r': out += 'r'; break;
	case '\t': out += 't'; break;
	case '\v': out += 'v'; break;

	case ' ': out += 's'; break;
		//no empty char ''!

	default:
		if (!isControlAscii(c))
			out += c;
		else
		{
			out += 'x';
			out += hexToChar(c >> 4);
			out += hexToChar(c & 0x0F);
		}
	}
}

void putContainerStart(StringBuilder& out, ConType con)
{
	switch (con)
	{
	case ConType::DOCUMENT:
		break;
	case ConType::DICTIONARY:
		out += OPEN_DICTIONARY;
		break;
	case ConType::LIST:
		out += OPEN_LIST;
		break;
	case ConType::TUPLE:
		out += OPEN_TUPLE;
		break;
	case ConType::FUNCTION_HEAD:
		out += OPEN_FUNCTION;
		break;
	default:
		throw domain_error("");
	}
}

void putContainerEnd(StringBuilder& out, ConType con)
{
	switch (con)
	{
	case ConType::DOCUMENT:
		break;
	case ConType::DICTIONARY:
		out += CLOSE_DICTIONARY;
		break;
	case ConType::LIST:
		out += CLOSE_LIST;
		break;
	case ConType::TUPLE:
		out += CLOSE_TUPLE;
		break;
	case ConType::FUNCTION_HEAD:
		out += CLOSE_FUNCTION;
		break;
	default:
		throw domain_error("");
	}
}

void Deserializer::putWebss(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.t)
	{
	case WebssType::NONE: case WebssType::DEFAULT:
		break;
	case WebssType::PRIMITIVE_NULL:
		out += 'N';
		break;
	case WebssType::PRIMITIVE_BOOL:
		out += webss.tBool ? 'T' : 'F';
		break;
	case WebssType::PRIMITIVE_INT:
		out += to_string(webss.tInt);
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		out += to_string(webss.tDouble);
		break;
	case WebssType::PRIMITIVE_STRING:
		putLineString(out, *webss.tString, con);
		break;
	case WebssType::DICTIONARY:
		putDictionary(out, *webss.dict);
		break;
	case WebssType::LIST:
		putList(out, *webss.list);
		break;
	case WebssType::TUPLE:
		putTuple(out, *webss.tuple);
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		putFheadBinary(out, *webss.fheadBinary);
		break;
	case WebssType::FUNCTION_HEAD_SCOPED:
		putFheadScoped(out, *webss.fheadScoped);
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		putFheadStandard(out, *webss.fheadStandard);
		break;
	case WebssType::FUNCTION_BINARY:
		putFuncBinary(out, *webss.funcBinary);
		break;
	case WebssType::FUNCTION_SCOPED:
		putFuncScoped(out, *webss.funcScoped, con);
		break;
	case WebssType::FUNCTION_STANDARD:
		putFuncStandard(out, *webss.funcStandard);
		break;
	case WebssType::ENTITY:
		putEntityName(out, webss.ent);
		break;
	case WebssType::NAMESPACE:
		putNamespace(out, *webss.nspace);
		break;
	case WebssType::ENUM:
		putEnum(out, *webss.nspace);
		break;
	case WebssType::BLOCK_HEAD:
		putBlockHead(out, *webss.blockHead);
		break;
	case WebssType::BLOCK:
		putBlock(out, *webss.block, con);
		break;
	default:
		throw domain_error("can't deserialize " + webss.t.toString());
	}
}

void Deserializer::putCharValue(StringBuilder& out, const Webss& value, ConType con)
{
	switch (value.t)
	{
	case WebssType::ENTITY:
		out += CHAR_EQUAL;
		putEntityName(out, value.ent);
		break;
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		out += CHAR_EQUAL;
		putWebss(out, value, con);
		break;
	case WebssType::PRIMITIVE_STRING:
		out += CHAR_COLON;
		putString(out, *value.tString, con);
		break;
	default:
		putWebss(out, value, con);
		break;
	}
}

void Deserializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	assert(value.t != WebssType::DEFAULT && ("can't deserialize " + value.t.toString() + " with key").c_str());

	out += key;
	putCharValue(out, value, con);
}

void Deserializer::putLineString(StringBuilder& out, const string& str, ConType con)
{
	out += CHAR_COLON;
	putString(out, str, con);
}

void Deserializer::putString(StringBuilder& out, const string& str, ConType con)
{
	if (str.empty())
		return;

	auto it = str.begin();
	//first char can't be ':'; first and last can't be ' '
	if (*it == ' ' || *it == ':')
	{
		addCharEscape(out, *it);
		if (++it == str.end())
			return;
	}

	do
	{
		if (isMustEscapeChar(*it) || isLineEnd(*it, con))
			addCharEscape(out, *it);
		else if (*it == '/') //escape comment
		{
			out += '/';
			if (++it == str.end())
				return;

			if (*it == '/' || *it == '*' || isMustEscapeChar(*it) || isLineEnd(*it, con))
				addCharEscape(out, *it);
			else
				out += *it;
		}
		else if (*it == ' ' && it + 1 == str.end())
			out += "\\s";
		else
			out += *it;
	} while (++it != str.end());
}

void Deserializer::putDictionary(StringBuilder& out, const Dictionary& dict)
{
	static const ConType::Enum CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it) { putKeyValue(out, it->first, it->second, CON); });
}

void Deserializer::putList(StringBuilder& out, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putString(out, it->getString(), CON); });
	}
	else
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putWebss(out, *it, CON); });
}

void Deserializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static const ConType::Enum CON = ConType::TUPLE;
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it) { putString(out, it->getString(), CON); });
	}
	else
	{
		using Type = decltype(tuple.getOrderedKeyValues());
		putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
		{
			if (it->first == nullptr)
				putWebss(out, *it->second, CON);
			else
				putKeyValue(out, *it->first, *it->second, CON);
		});
	}
}

void Deserializer::putDocument(StringBuilder& out, const Document& doc)
{
	static const ConType::Enum CON = ConType::DOCUMENT;
	/* put doc head...
	for (auto entName : locals)
	{
		const auto& content = ents[*entName].getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		putKeyValue(out, *entName, content, ConType::DOCUMENT);
		out += '\n';
	}*/
	
	using Type = decltype(doc.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, doc.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putWebss(out, *it->second, CON);
		else
			putKeyValue(out, *it->first, *it->second, CON);
	});
}

void Deserializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	static const ConType::Enum CON = ConType::DICTIONARY;
	currentNamespaces.insert(nspace.getPointer().get());
	putSeparatedValues<Namespace, CON>(out, nspace, [&](Namespace::const_iterator it) { putEntityDeclaration(out, *it, CON); });
	currentNamespaces.erase(nspace.getPointer().get());
}

void Deserializer::putEnum(StringBuilder& out, const Enum& tEnum)
{
	static const ConType::Enum CON = ConType::LIST;
	putSeparatedValues<Enum, CON>(out, tEnum, [&](Enum::const_iterator it) { putEntityName(out, *it); });
}

void Deserializer::putBlockHead(StringBuilder& out, const BlockHead& blockHead)
{
	out += OPEN_FUNCTION;
	if (blockHead.hasEntity())
		putEntityName(out, blockHead.getEntity());
	out += CLOSE_FUNCTION;
}

void Deserializer::putBlock(StringBuilder& out, const Block& block, ConType con)
{
	if (block.hasEntity())
	{
		putEntityName(out, block.getEntity());
		putCharValue(out, block.getValue(), con);
	}
	else
	{
		out += OPEN_FUNCTION;
		out += CLOSE_FUNCTION;
		putWebss(out, block.getValue(), con);
	}
}

void Deserializer::putEntityDeclaration(StringBuilder& out, const Entity& ent, ConType con)
{
	const auto& content = ent.getContent();
	out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
	putEntityName(out, ent);
	putKeyValue(out, "", content, con);
}

void Deserializer::putFuncStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Dictionary& dict)
{
	static const ConType::Enum CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncStandardList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncStandardTuple(out, params, it->second.getTuple());
		}
	});
}

void Deserializer::putFuncStandardList(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncStandardTuple(out, params, it->getTuple());
	});
}

void Deserializer::putFuncStandardTuple(StringBuilder& out, const FunctionHeadStandard::Tuple& params, const Tuple& tuple)
{
	static const ConType::Enum CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in function tuple");

	if (params.isText())
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it) { putString(out, it->getString(), CON); });
	else
	{
		decltype(params.size()) i = 0;
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
		{
			const auto& param = params[i++];
			if (!param.hasFunctionHead())
				putWebss(out, *it, CON);
			else
			{
				const auto& params2 = param.getFunctionHeadStandard().getParameters();
				switch (it->getType())
				{
				case WebssType::DICTIONARY:
					putFuncStandardDictionary(out, params2, it->getDictionary());
					break;
				case WebssType::LIST:
					putFuncStandardList(out, params2, it->getList());
					break;
				case WebssType::TUPLE:
					putFuncStandardTuple(out, params2, it->getTuple());
					break;
				default:
					assert(false && "function body must be dictionary, list or tuple");
				}
			}
		});
	}
}

void Deserializer::putFheadScoped(StringBuilder& out, const FunctionHeadScoped& fhead)
{
	static const ConType::Enum CON = ConType::FUNCTION_HEAD;
	if (fhead.hasEntity())
		putFheadEntity(out, fhead.getEntity());
	else
	{
		using Type = remove_reference<decltype(fhead.getParameters())>::type;
		putSeparatedValues<Type, CON>(out, fhead.getParameters(), [&](Type::const_iterator it)
		{
			if (it->hasEntity())
				putEntityDeclaration(out, it->getEntity(), CON);
			else
			{
				out += CHAR_USING_NAMESPACE;
				out += it->getNamespace().getName();
			}
		});
	}
}

#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
void Deserializer::putParamsStandard(StringBuilder& out, const FunctionHeadStandard& fhead, function<void(FUNC_PARAMS_STANDARD)> func)
{
	static const ConType::Enum CON = ConType::FUNCTION_HEAD;
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	using Type = decltype(keyValues);
	putSeparatedValues<Type, CON>(out, keyValues, [&](Type::const_iterator it)
	{
		assert(it->first != nullptr && ERROR_ANONYMOUS_KEY);
		func(*it->first, *it->second);
	});
}
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value
void Deserializer::putParamsBinary(StringBuilder& out, const FunctionHeadBinary& fhead, function<void(FUNC_PARAMS_BINARY)> func)
{
	static const ConType::Enum CON = ConType::FUNCTION_HEAD;
	auto keyValues = fhead.getParameters().getOrderedKeyValues();
	using Type = decltype(keyValues);
	putSeparatedValues<Type, CON>(out, keyValues, [&](Type::const_iterator it)
	{
		assert(it->first != nullptr && ERROR_ANONYMOUS_KEY);
		func(*it->first, *it->second);
	});
}

#undef FUNC_PARAMS_BINARY

void Deserializer::putFuncBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Dictionary& dict)
{
	static const ConType::Enum CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncBinaryList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncBinaryTuple(out, params, it->second.getTuple());
		}
	});
}

void Deserializer::putFuncBinaryList(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncBinaryTuple(out, params, it->getTuple());
	});
}

void Deserializer::putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& tuple)
{
	out += OPEN_TUPLE;
	putFuncBodyBinary(out, params, tuple);
	out += CLOSE_TUPLE;
}



#define FUNC_PARAMS_STANDARD const string& key, const ParamStandard& value
void Deserializer::putFheadStandard(StringBuilder& out, const FunctionHeadStandard& fhead)
{
	if (fhead.hasEntity())
		putFheadEntity(out, fhead.getEntity());
	else
	{
		assert(!fhead.empty() && "standard function head can't be empty");
		if (fhead.getParameters().isText())
		{
			out += ASSIGN_CONTAINER_STRING;
			putParamsStandard(out, fhead, [&](FUNC_PARAMS_STANDARD) { putParamText(out, key, value); });
		}
		else
			putParamsStandard(out, fhead, [&](FUNC_PARAMS_STANDARD) { putParamStandard(out, key, value); });
	}
}
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value
void Deserializer::putFheadBinary(StringBuilder& out, const FunctionHeadBinary& fhead)
{
	if (fhead.hasEntity())
		putFheadEntity(out, fhead.getEntity());
	else
	{
		assert(!fhead.empty() && "binary function head can't be empty");
		putParamsBinary(out, fhead, [&](FUNC_PARAMS_BINARY) { putParamBinary(out, key, value); });
	}
}
#undef FUNC_PARAMS_BINARY






void Deserializer::putFuncScoped(StringBuilder& out, const FunctionScoped& func, ConType con)
{
	//include the namespaces
	for (const auto& param : func.getParameters())
		if (param.hasNamespace())
			currentNamespaces.insert(param.getNamespace().getPointer().get());

	if (func.hasEntity())
	{
		putEntityName(out, func.getEntity());
		putCharValue(out, func.getValue(), con);
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




void Deserializer::putBinarySizeHead(StringBuilder& out, const ParamBinary::SizeHead& bhead)
{
	using Type = ParamBinary::SizeHead::Type;
	out += OPEN_TUPLE;
	if (bhead.getFlag() == ParamBinary::SizeHead::Flag::SELF)
		out += CHAR_SELF;

	switch (bhead.getType())
	{
	case Type::EMPTY:
		break;
	case Type::KEYWORD:
		out += bhead.getKeyword().toString();
		break;
	case Type::NUMBER:
		out += to_string(bhead.size());
		break;
	case Type::FUNCTION_HEAD:
		putFheadBinary(out, bhead.getFunctionHead());
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		putEntityName(out, bhead.getEntityNumber());
		break;
	case Type::ENTITY_FUNCTION_HEAD:
		putEntityName(out, bhead.getEntityFunctionHead());
		break;
	default:
		throw domain_error("");
	}
	out += CLOSE_TUPLE;
}

void Deserializer::putBinarySizeList(StringBuilder& out, const ParamBinary::SizeList& blist)
{
	using Type = ParamBinary::SizeList::Type;
	if (blist.isOne())
		return;

	out += OPEN_LIST;
	switch (blist.getType())
	{
	case Type::EMPTY:
		break;
	case Type::NUMBER:
		out += to_string(blist.size());
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		putEntityName(out, blist.getEntity());
		break;
	default:
		throw domain_error("");
	}
	out += CLOSE_LIST;
}