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

string webss::deserializeAll(const Document& doc)
{
	StringBuilder out;
	putDocument(out, doc);
	return out;
}

string webss::deserializeAll(const Document& doc, const EntityManager& ents)
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

void webss::putWebss(StringBuilder& out, const Webss& webss, ConType con)
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
	case WebssType::FUNCTION_STANDARD:
		putFuncStandard(out, *webss.funcStandard);
		break;
	case WebssType::FUNCTION_BINARY:
		putFuncBinary(out, *webss.funcBinary);
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		putFheadStandard(out, *webss.fheadStandard);
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		putFheadBinary(out, *webss.fheadBinary);
		break;
	case WebssType::ENTITY:
		out += webss.ent.getName();
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

void webss::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	switch (value.t)
	{
	case WebssType::DEFAULT:
		throw domain_error("can't deserialize " + value.t.toString() + " with key");
	case WebssType::ENTITY:
		out += key + CHAR_EQUAL + value.ent.getName();
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


void webss::putValueOnly(StringBuilder& out, const Webss& value, ConType con)
{
	putWebss(out, value, con);
}


void webss::putSeparatedValues(StringBuilder& out, function<bool()> condition, function<void()> output)
{
	output();
	while (condition())
	{
		out += CHAR_SEPARATOR;
		output();
	}
}

void webss::putLineString(StringBuilder& out, const string& str, ConType con)
{
	out += CHAR_COLON;
	putString(out, str, con);
}

void webss::putString(StringBuilder& out, const string& str, ConType con)
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

void webss::putDictionary(StringBuilder& out, const Dictionary& dict)
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

void webss::putList(StringBuilder& out, const List& list)
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

void webss::putTuple(StringBuilder& out, const Tuple& tuple)
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

void webss::putDocument(StringBuilder& out, const Document& doc)
{
	if (doc.empty())
		return;

	auto keyValues = doc.getOrderedKeyValues();
	auto it = keyValues.begin();
	putSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? putValueOnly(out, *it->second, ConType::DOCUMENT) : putKeyValue(out, *it->first, *it->second, ConType::DOCUMENT); });
}

void webss::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	if (nspace.empty())
	{
		out += EMPTY_DICTIONARY;
		return;
	}

	auto it = nspace.begin();
	out += OPEN_DICTIONARY;
	putSeparatedValues(out, [&]() { return ++it != nspace.end(); }, [&]()
	{
		const auto& content = it->second.getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		putKeyValue(out, it->second.getName(), content, ConType::DICTIONARY);
	});
	out += CLOSE_DICTIONARY;
}

void webss::putEnum(StringBuilder& out, const Enum& tEnum)
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

void webss::putBlockHead(StringBuilder& out, const BlockHead& blockHead)
{
	if (blockHead.hasEntity())
		out += OPEN_FUNCTION + blockHead.getEntName() + CLOSE_FUNCTION;
	else
		out += EMPTY_FUNCTION;
}

void webss::putBlock(StringBuilder& out, const Block& block, ConType con)
{
	if (block.hasEntity())
		putKeyValue(out, block.getEntName(), block.getValue(), con);
	else
	{
		out += EMPTY_FUNCTION;
		putWebss(out, block.getValue(), con);
	}
}