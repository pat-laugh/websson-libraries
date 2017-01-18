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
	deserializeDocument(out, doc);
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

	deserializeDocument(out, doc);
	return out;
}

void webss::deserializeWebss(StringBuilder& out, const Webss& webss)
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
	case WebssType::DICTIONARY:
		deserializeDictionary(out, *webss.dict);
		break;
	case WebssType::LIST:
		deserializeList(out, *webss.list);
		break;
	case WebssType::TUPLE:
		deserializeTuple(out, *webss.tuple);
		break;
	case WebssType::FUNCTION_STANDARD:
		deserializeFunctionStandard(out, *webss.funcStandard);
		break;
	case WebssType::FUNCTION_BINARY:
		deserializeFunctionBinary(out, *webss.funcBinary);
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		deserializeFunctionHeadStandard(out, *webss.fheadStandard);
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		deserializeFunctionHeadBinary(out, *webss.fheadBinary);
		break;
	case WebssType::BLOCK_VALUE:
		deserializeBlock(out, *webss.block);
		break;
	case WebssType::VARIABLE:
		out += webss.ent.getName();
		break;
	case WebssType::NAMESPACE:
		putNamespace(out, *webss.nspace);
		break;
	case WebssType::ENUM:
		putEnum(out, *webss.nspace);
		break;
	case WebssType::PRIMITIVE_STRING: //type of string should be checked before calling function
		throw domain_error("can't deserialize string without its container's type");
	default:
		throw domain_error("can't deserialize " + webss.t.toString());
	}
}

void webss::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType stringCon)
{
	switch (value.t)
	{
	case WebssType::DEFAULT:
		throw domain_error("can't deserialize " + value.t.toString() + " with key");
	case WebssType::VARIABLE:
		out += key + CHAR_EQUAL + value.ent.getName();
		break;
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		out += key + CHAR_EQUAL;
		deserializeWebss(out, value);
		break;
	case WebssType::PRIMITIVE_STRING:
		out += key + CHAR_COLON;
		deserializeString(out, *value.tString, stringCon);
		break;
	case WebssType::BLOCK_ID:
		out += key + OPEN_TUPLE + (value.getBlockId().hasEntity() ? value.getBlockId().getEntName() : to_string(value.getBlockId().getIndex()));
		break;
	default:
		out += key;
		deserializeWebss(out, value);
		break;
	}
}


void webss::getValueOnly(StringBuilder& out, const Webss& value, ConType stringCon)
{
	if (value.t != WebssType::PRIMITIVE_STRING)
		deserializeWebss(out, value);
	else
	{
		out += ':';
		deserializeString(out, *value.tString, stringCon);
	}
}


void webss::getSeparatedValues(StringBuilder& out, function<bool()> condition, function<void()> output)
{
	output();
	while (condition())
	{
		out += CHAR_SEPARATOR;
		output();
	}
}

void webss::deserializeString(StringBuilder& out, const string& str, ConType con)
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

void webss::deserializeDictionary(StringBuilder& out, const Dictionary& dict)
{
	if (dict.empty())
	{
		out += EMPTY_DICTIONARY;
		return;
	}

	auto it = dict.begin();
	out += OPEN_DICTIONARY;
	getSeparatedValues(out, [&]() { return ++it != dict.end(); }, [&]() { putKeyValue(out, it->first, it->second, ConType::DICTIONARY); });
	out += CLOSE_DICTIONARY;
}

void webss::deserializeList(StringBuilder& out, const List& list)
{
	if (list.containerText)
	{
		out += ASSIGN_CONTAINER_STRING;
		if (list.empty())
		{
			out += EMPTY_LIST;
			return;
		}
		auto it = list.begin();
		out += OPEN_LIST;
		getSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]() { deserializeString(out, it->getString(), ConType::LIST); });
		out += CLOSE_LIST;
	}

	if (list.empty())
	{
		out += EMPTY_LIST;
		return;
	}
	auto it = list.begin();
	out += OPEN_LIST;
	getSeparatedValues(out, [&]() { return ++it != list.end(); }, [&]()
	{
		if (it->t != WebssType::PRIMITIVE_STRING)
			deserializeWebss(out, *it);
		else
		{
			out += ':';
			deserializeString(out, *it->tString, ConType::LIST);
		}
	});
	out += CLOSE_LIST;
}

void webss::deserializeTuple(StringBuilder& out, const Tuple& tuple)
{
	if (tuple.containerText)
	{
		out += ASSIGN_CONTAINER_STRING;
		if (tuple.empty())
		{
			out += EMPTY_TUPLE;
			return;
		}
		auto it = tuple.begin();
		out += OPEN_TUPLE;
		getSeparatedValues(out, [&]() { return ++it != tuple.end(); }, [&]() { deserializeString(out, it->getString(), ConType::TUPLE); });
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
	getSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? getValueOnly(out, *it->second, ConType::TUPLE) : putKeyValue(out, *it->first, *it->second, ConType::TUPLE); });
	out += CLOSE_TUPLE;
}

void webss::deserializeDocument(StringBuilder& out, const Document& doc)
{
	if (doc.empty())
		return;

	auto keyValues = doc.getOrderedKeyValues();
	auto it = keyValues.begin();
	getSeparatedValues(out, [&]() { return ++it != keyValues.end(); }, [&]() { it->first == nullptr ? getValueOnly(out, *it->second, ConType::DOCUMENT) : putKeyValue(out, *it->first, *it->second, ConType::DOCUMENT); });
}

void webss::deserializeBlock(StringBuilder& out, const Block& block)
{
	out += block.getName();
	deserializeWebss(out, block.getValue());
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
	getSeparatedValues(out, [&]() { return ++it != nspace.end(); }, [&]()
	{
		const auto& content = it->getContent();
		out += content.isConcrete() ? CHAR_CONCRETE_ENTITY : CHAR_ABSTRACT_ENTITY;
		out += it->getName();
		deserializeWebss(out, content);
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
	for (const auto& ent : tEnum)
		elems[static_cast<Enum::size_type>(ent.getContent().getInt())] = const_cast<string*>(&ent.getName());

	auto it = elems.begin();
	out += OPEN_LIST;
	getSeparatedValues(out, [&]() { return ++it != elems.end(); }, [&]()
	{
		out += **it;
	});
	out += CLOSE_LIST;
}