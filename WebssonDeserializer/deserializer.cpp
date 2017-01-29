//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"
#include <type_traits>

using namespace std;
using namespace webss;

void putContainerStart(StringBuilder& out, ConType con);
void putContainerEnd(StringBuilder& out, ConType con);

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

	template <ConType::Enum CON>
	void putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
	{
		putSeparatedValues<DocumentHead, CON>(out, docHead, [&](DocumentHead::const_iterator it)
		{
			using Type = decltype(it->getType());
			switch (it->getType())
			{
			case Type::ENTITY_ABSTRACT:
				putEntityDeclaration(out, it->getAbstractEntity(), CON);
				break;
			case Type::ENTITY_CONCRETE:
				putEntityDeclaration(out, it->getConcreteEntity(), CON);
				break;
			case Type::IMPORT:
				putImportedDocument(out, it->getImportedDoc(), CON);
				break;
			case Type::SCOPED_DOCUMENT:
				putScopedDocument(out, it->getScopedDoc());
				break;
			default:
				break;
			}
		});
	}
};

#define putEntityName static_cast<DeserializerTemplate*>(this)->putEntityName
#define putFheadEntity static_cast<DeserializerTemplate*>(this)->putFheadEntity

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
		assert(false);
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
		assert(false);
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
		out += CHAR_COLON;
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
	case WebssType::FUNCTION_HEAD_TEXT:
		putFheadText(out, *webss.fheadText);
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
	case WebssType::FUNCTION_TEXT:
		putFuncText(out, *webss.funcText);
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
		assert(false && "can't deserialize this type");
	}
}

void Deserializer::putConcreteValue(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.t)
	{
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
		out += CHAR_COLON;
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
	case WebssType::FUNCTION_BINARY:
		putFuncBinary(out, *webss.funcBinary);
		break;
	case WebssType::FUNCTION_SCOPED:
		putFuncScoped(out, *webss.funcScoped, con);
		break;
	case WebssType::FUNCTION_STANDARD:
		putFuncStandard(out, *webss.funcStandard);
		break;
	case WebssType::FUNCTION_TEXT:
		putFuncText(out, *webss.funcText);
		break;
	case WebssType::ENTITY:
		assert(webss.ent.getContent().isConcrete());
		putEntityName(out, webss.ent);
		break;
	case WebssType::BLOCK:
		putBlock(out, *webss.block, con);
		break;
	default:
		assert(false && "type is not a value-only");
	}
}

void Deserializer::putCharValue(StringBuilder& out, const Webss& value, ConType con)
{
	switch (value.t)
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT:
	case WebssType::PRIMITIVE_DOUBLE: case WebssType::ENTITY:
		out += CHAR_EQUAL;
	default:
		putConcreteValue(out, value, con);
		break;
	}
}

void Deserializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	assert(value.t != WebssType::DEFAULT && "can't deserialize this type with key");

	out += key;
	putCharValue(out, value, con);
}

void Deserializer::putLineString(StringBuilder& out, const string& str, ConType con)
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

void Deserializer::putCstring(StringBuilder& out, const string& str)
{
	out += CHAR_CSTRING;
	if (!str.empty())
		for (auto it = str.begin(); it != str.end(); ++it)
			if (isMustEscapeChar(*it) || *it == CHAR_CSTRING)
				addCharEscape(out, *it);
			else
				out += *it;
	out += CHAR_CSTRING;
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
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putLineString(out, it->getString(), CON); });
	}
	else
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putConcreteValue(out, *it, CON); });
}

void Deserializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static const ConType::Enum CON = ConType::TUPLE;
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it) { putLineString(out, it->getString(), CON); });
	}
	else
	{
		using Type = decltype(tuple.getOrderedKeyValues());
		putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
		{
			if (it->first == nullptr)
				putConcreteValue(out, *it->second, CON);
			else
				putKeyValue(out, *it->first, *it->second, CON);
		});
	}
}

void Deserializer::putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
{
	static_cast<DeserializerTemplate*>(this)->putDocumentHead<ConType::DOCUMENT>(out, docHead);
}

void Deserializer::putDocument(StringBuilder& out, const Document& doc)
{
	static const ConType::Enum CON = ConType::DOCUMENT;

	putDocumentHead(out, doc.getHead());
	out += '\n';
	
	using Type = decltype(doc.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, doc.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putWebss(out, *it->second, CON);
		else
			putKeyValue(out, *it->first, *it->second, CON);
	});
}

void Deserializer::putImportedDocument(StringBuilder& out, const ImportedDocument& importDoc, ConType con)
{
	out += CHAR_IMPORT;
	const auto& name = importDoc.getName();
	assert(name.isString());
	if (name.t == WebssType::PRIMITIVE_STRING)
		putCstring(out, *name.tString);
	else
		putConcreteValue(out, importDoc.getName(), con);
}

void Deserializer::putScopedDocument(StringBuilder& out, const ScopedDocument& scopedDoc)
{
	out += CHAR_USING_NAMESPACE;
	putFheadScoped(out, scopedDoc.head);

	//include the namespaces
	const auto& params = scopedDoc.head.getParameters();
	for (const auto& param : params)
		if (param.hasNamespace())
			currentNamespaces.insert(param.getNamespace().getPointer().get());

	static_cast<DeserializerTemplate*>(this)->putDocumentHead<ConType::DICTIONARY>(out, scopedDoc.body);

	//remove the namespaces
	for (const auto& param : params)
		if (param.hasNamespace())
			currentNamespaces.erase(param.getNamespace().getPointer().get());
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
		putConcreteValue(out, block.getValue(), con);
	}
}

void Deserializer::putEntityDeclaration(StringBuilder& out, const Entity& ent, ConType con)
{
	const auto& content = ent.getContent();
	out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
	putEntityName(out, ent);
	putCharValue(out, content, con);
}

void Deserializer::putFuncStandardDictionary(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const Dictionary& dict)
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

void Deserializer::putFuncStandardList(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
		{
			assert(it->isTuple());
			putFuncStandardTupleText(out, params, it->getTuple());
		});
	}
	else
	{
		putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
		{
			assert(it->isTuple());
			putFuncStandardTuple(out, params, it->getTuple());
		});
	}
}

void Deserializer::putFuncStandardTuple(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const Tuple& tuple)
{
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putFuncStandardTupleText(out, params, tuple);
		return;
	}

	static const ConType::Enum CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in function tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		if (!param.hasFunctionHead())
		{
			if (it->t == WebssType::NONE || it->t == WebssType::DEFAULT)
				assert(param.hasDefaultValue());
			else
				putConcreteValue(out, *it, CON);
		}
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

void Deserializer::putFuncStandardTupleText(StringBuilder& out, const FunctionHeadStandard::Parameters& params, const Tuple& tuple)
{
	static const ConType::Enum CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in function tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		assert(!param.hasFunctionHead());
		switch (it->t)
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, *it->tString, CON);
			break;
		default:
			assert(false);
			throw logic_error("");
		}
	});
#ifdef assert
	while (i < params.size())
		assert(params[i++].hasDefaultValue());
#endif
}

void Deserializer::putFuncTextDictionary(StringBuilder& out, const FunctionHeadText::Parameters& params, const Dictionary& dict)
{
	static const ConType::Enum CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it)
	{
		out += it->first;
		if (it->second.isList())
			putFuncTextList(out, params, it->second.getList());
		else
		{
			assert(it->second.isTuple());
			putFuncTextTuple(out, params, it->second.getTuple());
		}
	});
}

void Deserializer::putFuncTextList(StringBuilder& out, const FunctionHeadText::Parameters& params, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncTextTuple(out, params, it->getTuple());
	});
}

void Deserializer::putFuncTextTuple(StringBuilder& out, const FunctionHeadText::Parameters& params, const Tuple& tuple)
{
	static const ConType::Enum CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in function tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		switch (it->t)
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, *it->tString, CON);
			break;
		default:
			assert(false);
			throw logic_error("");
		}
	});
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
			using ParamType = decltype(it->getType());
			switch (it->getType())
			{
			case ParamType::ENTITY_ABSTRACT: 
				putEntityDeclaration(out, it->getAbstractEntity(), CON);
				break;
			case ParamType::ENTITY_CONCRETE:
				putEntityDeclaration(out, it->getConcreteEntity(), CON);
				break;
			case ParamType::NAMESPACE:
				out += CHAR_USING_NAMESPACE;
				out += it->getNamespace().getName();
				break;
			case ParamType::IMPORT:
				putImportedDocument(out, it->getImportedDoc(), CON);
				break;
			case ParamType::SCOPED_DOCUMENT:
				putScopedDocument(out, it->getScopedDoc());
				break;
			default:
				assert(false);
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

#define FUNC_PARAMS_TEXT const string& key, const ParamText& value
void Deserializer::putParamsText(StringBuilder& out, const FunctionHeadText& fhead, function<void(FUNC_PARAMS_TEXT)> func)
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
#undef FUNC_PARAMS_TEXT

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

void Deserializer::putFuncBinaryDictionary(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Dictionary& dict)
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

void Deserializer::putFuncBinaryList(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const List& list)
{
	static const ConType::Enum CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it)
	{
		assert(it->isTuple());
		putFuncBinaryTuple(out, params, it->getTuple());
	});
}

void Deserializer::putFuncBinaryTuple(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Tuple& tuple)
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
		putParamsStandard(out, fhead, [&](FUNC_PARAMS_STANDARD) { putParamStandard(out, key, value); });
	}
}
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_TEXT const string& key, const ParamText& value
void Deserializer::putFheadText(StringBuilder& out, const FunctionHeadText& fhead)
{
	if (fhead.hasEntity())
		putFheadEntity(out, fhead.getEntity());
	else
	{
		assert(!fhead.empty() && "text function head can't be empty");
		out += ASSIGN_CONTAINER_STRING;
		putParamsText(out, fhead, [&](FUNC_PARAMS_TEXT) { putParamText(out, key, value); });
	}
}
#undef FUNC_PARAMS_TEXT

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
		putConcreteValue(out, func.getValue(), con);
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
	case Type::SELF:
		out += "(&)";
		break;
	default:
		assert(false);
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
		assert(false);
	}
	out += CLOSE_LIST;
}