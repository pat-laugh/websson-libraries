//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

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

string Deserializer::deserialize(const Document& doc)
{
	StringBuilder out;
	putDocument(out, doc);
	return out;
}

string Deserializer::deserialize(const Document& doc, const EntityManager& ents)
{
	StringBuilder out;

	//first put local entities
	auto locals = ents.getOrderedLocals();
	for (auto entName : locals)
	{
		const auto& content = ents[*entName].getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		putKeyValue(out, *entName, content, ConType::DOCUMENT);
		out += '\n';
	}

	putDocument(out, doc);
	return out;
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
//	case WebssType::PRIMITIVE_STRING: //type of string should be checked before calling function
//		throw domain_error("can't deserialize string without its container's type");
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
	if (dict.empty())
	{
		out += EMPTY_DICTIONARY;
		return;
	}

	auto it = dict.begin();
	out += OPEN_DICTIONARY;
	putSeparatedValues(out, [&]() { return ++it != dict.end(); }, [&]() { putKeyValue(out, it->first, it->second, ConType::DICTIONARY); });
	out += CLOSE_DICTIONARY;
}

void Deserializer::putList(StringBuilder& out, const List& list)
{
	if (list.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		if (list.empty())
		{
			out += EMPTY_LIST;
			return;
		}
		auto it = list.begin();
		out += OPEN_LIST;
		putSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]() { putString(out, it->getString(), ConType::LIST); });
		out += CLOSE_LIST;
	}

	if (list.empty())
	{
		out += EMPTY_LIST;
		return;
	}
	auto it = list.begin();
	out += OPEN_LIST;
	putSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]() { putWebss(out, *it, ConType::LIST); });
	out += CLOSE_LIST;
}

void Deserializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	if (tuple.isText())
	{
		out += ASSIGN_CONTAINER_STRING;
		if (tuple.empty())
		{
			out += EMPTY_TUPLE;
			return;
		}
		auto it = tuple.begin();
		out += OPEN_TUPLE;
		putSeparatedValues(out, [&]() { return ++it != tuple.end(); }, [&]() { putString(out, it->getString(), ConType::TUPLE); });
		out += CLOSE_TUPLE;
	}
	if (tuple.empty())
	{
		out += EMPTY_TUPLE;
		return;
	}

	auto keyValues = tuple.getOrderedKeyValues();
	auto it = keyValues.begin();
	out += OPEN_TUPLE;
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? putValueOnly(out, *it->second, ConType::TUPLE) : putKeyValue(out, *it->first, *it->second, ConType::TUPLE); });
	out += CLOSE_TUPLE;
}

void Deserializer::putDocument(StringBuilder& out, const Document& doc)
{
	if (doc.empty())
		return;

	auto keyValues = doc.getOrderedKeyValues();
	auto it = keyValues.begin();
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? putValueOnly(out, *it->second, ConType::DOCUMENT) : putKeyValue(out, *it->first, *it->second, ConType::DOCUMENT); });
}

void Deserializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	if (nspace.empty())
	{
		out += EMPTY_DICTIONARY;
		return;
	}

	currentNamespaces.insert(nspace.getPointer().get());

	auto it = nspace.begin();
	out += OPEN_DICTIONARY;
	putSeparatedValues(out, [&]() { return ++it != nspace.end(); }, [&]()
	{
		const auto& content = it->second.getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		putEntityName(out, it->second);
		putKeyValue(out, "", content, ConType::DICTIONARY);
	});
	out += CLOSE_DICTIONARY;

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