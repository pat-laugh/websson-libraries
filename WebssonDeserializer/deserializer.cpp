//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

template <class T, const ConType CON>
void putSeparatedValues(StringBuilder& out, const T& t, function<void(typename T::iterator it)> output)
{
	putContainerStart(out, CON);
	if (!t.empty())
	{
		auto it = t.begin();
		output();
		while (++it != t.end())
		{
			out += CHAR_SEPARATOR;
			output();
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

putContainerStart(StringBuilder& out, ConType con)
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
putContainerEnd(StringBuilder& out, ConType con)
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

void Deserializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	switch (value.t)
	{
	case WebssType::DEFAULT:
		throw domain_error("can't deserialize " + value.t.toString() + " with key");
	case WebssType::ENTITY:
		out += key + CHAR_EQUAL;
		putEntityName(out, value.ent);
		break;
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		out += key + CHAR_EQUAL;
		putWebss(out, value, con);
		break;
	default:
		out += key;
		putWebss(out, value, con);
		break;
	}
}

void Deserializer::putValueOnly(StringBuilder& out, const Webss& value, ConType con)
{
	putWebss(out, value, con);
}

void Deserializer::putSeparatedValues(StringBuilder& out, function<bool()> condition, function<void()> output)
{
	output();
	while (condition())
	{
		out += CHAR_SEPARATOR;
		output();
	}
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
	static constexpr ConType CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::iterator it) { putKeyValue(out, it->first, it->second, CON); });
}

void Deserializer::putList(StringBuilder& out, const List& list)
{
	static constexpr ConType CON = ConType::LIST;
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<List, CON>(out, list, [&](List::iterator it) { putString(out, it->getString(), CON); });
	}
	else
		putSeparatedValues<List, CON>(out, list, [&](List::iterator it) { putWebss(out, *it, CON); });
}

void Deserializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static constexpr ConType CON = ConType::TUPLE;
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::iterator it) { putString(out, it->getString(), CON); });
	}
	else
		putSeparatedValues<vector, CON>(out, tuple.getOrderedKeyValues(), [&](Tuple::iterator it)
		{
			if (it->first == nullptr)
				putValueOnly(out, *it->second, CON)
			else
				putKeyValue(out, *it->first, *it->second, CON);
		});
}

void Deserializer::putDocument(StringBuilder& out, const Document& doc)
{
	/* put doc head...
	for (auto entName : locals)
	{
		const auto& content = ents[*entName].getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		putKeyValue(out, *entName, content, ConType::DOCUMENT);
		out += '\n';
	}*/
	
	if (doc.empty())
		return;

	auto keyValues = doc.getOrderedKeyValues();
	auto it = keyValues.begin();
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? putValueOnly(out, *it->second, ConType::DOCUMENT) : putKeyValue(out, *it->first, *it->second, ConType::DOCUMENT); });
}

void Deserializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	static constexpr ConType CON = ConType::DICTIONARY;
	currentNamespaces.insert(nspace.getPointer().get());
	putSeparatedValues<Namespace, CON>(out, nspace, [&]() { putEntityDeclaration(out, it->second, CON); });
	currentNamespaces.erase(nspace.getPointer().get());
}

void Deserializer::putEnum(StringBuilder& out, const Enum& tEnum)
{
	if (tEnum.empty())
	{
		out += EMPTY_LIST;
		return;
	}

	//sort the enum...
	vector<string*> elems(tEnum.size());
	for (const auto& entPair : tEnum)
		elems[static_cast<Enum::size_type>(entPair.second.getContent().getInt())] = const_cast<string*>(&entPair.second.getName());

	auto it = elems.begin();
	out += OPEN_LIST;
	putSeparatedValues(out, [&]() { return ++it != elems.end(); }, [&]()
	{
		out += **it;
	});
	out += CLOSE_LIST;
}

void Deserializer::putBlockHead(StringBuilder& out, const BlockHead& blockHead)
{
	if (blockHead.hasEntity())
	{
		out += OPEN_FUNCTION;
		putEntityName(out, blockHead.getEntity());
		out += CLOSE_FUNCTION;
	}
	else
		out += EMPTY_FUNCTION;
}

void Deserializer::putBlock(StringBuilder& out, const Block& block, ConType con)
{
	if (block.hasEntity())
	{
		putEntityName(out, block.getEntity());
		putKeyValue(out, "", block.getValue(), con);
	}
	else
	{
		out += EMPTY_FUNCTION;
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

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<Webss>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<BlockHead>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<FunctionHeadBinary>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<FunctionHeadScoped>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<FunctionHeadStandard>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<WebssBinarySize>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder&out, const string& entName, const BasicNamespace<WebssInt>& entNspace)
{
	const auto& nspaces = entNspace.getNamespaces();
	int i = 0;
	for (; i < nspaces.size(); ++i)
		if (currentNamespaces.find(reinterpret_cast<Namespace*>(nspaces[i].get())) == currentNamespaces.end())
			break;

	for (; i < nspaces.size(); ++i)
		out += nspaces[i]->getName() + CHAR_SCOPE;

	out += entName;
}

void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<Webss>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}

void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<BlockHead>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}
void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<FunctionHeadBinary>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}
void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<FunctionHeadScoped>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}
void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<FunctionHeadStandard>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}
void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<WebssBinarySize>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
}
void Deserializer::putEntityName(StringBuilder& out, const BasicEntity<WebssInt>& ent)
{
	if (ent.hasNamespace())
		putEntityName(out, ent.getName(), ent.getNamespace());
	else
		out += ent.getName();
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
	assert(tuple.size() <= params.size() && "too many elements in function tuple");
	
	if (tuple.empty())
	{
		out += EMPTY_TUPLE;
		return;
	}

	out += OPEN_TUPLE;
	if (params.isText())
	{
		auto it = tuple.begin();
		putSeparatedValues(out, [&]() { return it != tuple.end(); }, [&]()
		{
			putString(out, it->getString(), ConType::TUPLE);
		});
	}
	else
	{
		Tuple::size_type i = 0;
		putSeparatedValues(out, [&]() { ++i; return it != tuple.end(); }, [&]()
		{
			if (params[i].hasFunctionHead())
			{
				const auto& parameters2 = params[i].getFunctionHeadStandard().getParameters();
				if (it->isList())
					putFuncStandardList(out, parameters2, it->getList());
				else if (it->isTuple())
					putFuncStandardTuple(out, parameters2, it->getTuple());
				else
					throw logic_error("implementation of function must be a list or tuple");
				return;
			}

			const Webss& webss = *it;
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
		const auto& params = fhead.getParameters();
		auto it = params.begin();
		putSeparatedValues(out, [&]() { return ++it != params.end(); }, [&]()
		{
			if (it->hasEntity())
				putEntityDeclaration(out, it->getEntity(), ConType::FUNCTION_HEAD);
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
#undef FUNC_PARAMS_STANDARD

#define FUNC_PARAMS_BINARY const string& key, const ParamBinary& value
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