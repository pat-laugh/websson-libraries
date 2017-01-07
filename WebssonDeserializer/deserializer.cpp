//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void addCharEscape(string& str, char c)
{
	str += '\\';
	switch (c)
	{
	case '\0': str += '0'; break;
	case '\a': str += 'a'; break;
	case '\b': str += 'b'; break;
	case '\f': str += 'f'; break;
	case '\n': str += 'n'; break;
	case '\r': str += 'r'; break;
	case '\t': str += 't'; break;
	case '\v': str += 'v'; break;

	case ' ': str += 's'; break;
		//no empty char ''!

	default:
		if (!isControlAscii(c))
			str += c;
		else
		{
			str += 'x';
			str += hexToChar(c >> 4);
			str += hexToChar(c & 0x0F);
		}
	}
}

string webss::deserializeAll(const Document& doc)
{
	return deserializeDocument(doc);
}

string webss::deserializeAll(const Document& doc, const VariablesManager& vars)
{
	string out;

	//first put local variables
	auto locals = vars.getOrderedLocals();
	for (auto varName : locals)
	{
		const auto& content = vars[*varName].getContent();
//		out += content.t == WebssType::VARIABLE_KEYWORD ? CHAR_BLOCK : CHAR_VARIABLE;
		out += getKeyValue(*varName, content, ConType::DOCUMENT) + '\n';
	}

	return out + deserializeDocument(doc);
}

string webss::deserializeWebss(const Webss& webss)
{
	switch (webss.t)
	{
	case WebssType::NONE: case WebssType::DEFAULT:
		return "";
	case WebssType::PRIMITIVE_NULL:
		return "N";
	case WebssType::PRIMITIVE_BOOL:
		return webss.tBool ? "T" : "F";
	case WebssType::PRIMITIVE_INT:
		return to_string(webss.tInt);
	case WebssType::PRIMITIVE_DOUBLE:
		return to_string(webss.tDouble);
	case WebssType::DICTIONARY:
		return deserializeDictionary(*webss.dictionary);
	case WebssType::LIST:
		return deserializeList(*webss.list);
	case WebssType::TUPLE:
		return deserializeTuple(*webss.tuple);
	case WebssType::FUNCTION_STANDARD:
		return deserializeFunctionStandard(*webss.funcStandard);
	case WebssType::FUNCTION_BINARY:
		return deserializeFunctionBinary(*webss.funcBinary);
	case WebssType::FUNCTION_HEAD_STANDARD:
		return deserializeFunctionHeadStandard(*webss.fheadStandard);
	case WebssType::FUNCTION_HEAD_BINARY:
		return deserializeFunctionHeadBinary(*webss.fheadBinary);
	case WebssType::BLOCK_VALUE:
		return deserializeBlock(*webss.block);
	case WebssType::VARIABLE:
		return webss.var.getName();
	case WebssType::PRIMITIVE_STRING: //type of string should be checked before calling function
		throw domain_error("can't deserialize string without its container's type");
	default:
		throw domain_error("can't deserialize " + webss.t.toString());
	}
}

string webss::getKeyValue(const string& key, const Webss& value, ConType stringCon)
{
	switch (value.t)
	{
	case WebssType::DEFAULT:
		throw domain_error("can't deserialize " + value.t.toString() + " with key");
	case WebssType::VARIABLE:
		return key + CHAR_EQUAL + value.var.getName();
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		return key + CHAR_EQUAL + deserializeWebss(value);
	case WebssType::PRIMITIVE_STRING:
		return key + CHAR_COLON + deserializeString(*value.tString, stringCon);
//	case WebssType::VARIABLE_KEYWORD:
//		return key + CHAR_EQUAL + to_string(value.tInt);
	default:
		return key + deserializeWebss(value);
	}
}


string webss::getValueOnly(const Webss& value, ConType stringCon)
{
	return value.t == WebssType::PRIMITIVE_STRING ? ':' + deserializeString(*value.tString, stringCon) : deserializeWebss(value);
}


string webss::getSeparatedValues(function<bool()> condition, function<string()> output)
{
	string out;
	out += output();
	while (condition())
		out += CHAR_SEPARATOR + output();
	return out;
}

string webss::deserializeString(const string& str, ConType con)
{
	if (str.empty())
		return "";
	string out;
	out.reserve(str.size());
	auto it = str.begin();
	//first char can't be ':'; first and last can't be ' '
	if (*it == ' ' || *it == ':')
	{
		addCharEscape(out, *it);
		if (++it == str.end())
			return out;
	}

	do
	{
		if (isMustEscapeChar(*it) || isLineEnd(*it, con))
			addCharEscape(out, *it);
		else if (*it == '/') //escape comment
		{
			out += '/';
			if (++it == str.end())
				return out;

			if (*it == '/' || *it == '*' || isMustEscapeChar(*it) || isLineEnd(*it, con))
				addCharEscape(out, *it);
			else
				out += *it;
		}
		else
			out += *it;
	} while (++it != str.end());

	//make sure last isn't ' '
	if (out.back() == ' ')
	{
		out.back() = '\\';
		out += 's';
	}

	return out;
}

string webss::deserializeDictionary(const Dictionary& dictionary)
{
	if (dictionary.empty())
		return EMPTY_DICTIONARY;

	auto it = dictionary.begin();
	return OPEN_DICTIONARY + getSeparatedValues([&]() { return ++it != dictionary.end(); }, [&]() { return getKeyValue(it->first, it->second, ConType::DICTIONARY); }) + CLOSE_DICTIONARY;
}

string webss::deserializeList(const List& list)
{
	if (list.containerText)
	{
		string out(ASSIGN_CONTAINER_STRING);
		if (list.empty())
			return out + EMPTY_LIST;
		auto it = list.begin();
		return out + OPEN_LIST + getSeparatedValues([&]() { return ++it != list.end(); }, [&]() { return deserializeString(it->getString(), ConType::LIST); }) + CLOSE_LIST;
	}
	if (list.empty())
		return EMPTY_LIST;

	auto it = list.begin();
	return OPEN_LIST + getSeparatedValues([&]() { return ++it != list.end(); }, [&]() { return it->t == WebssType::PRIMITIVE_STRING ? ':' + deserializeString(*it->tString, ConType::LIST) : deserializeWebss(*it); }) + CLOSE_LIST;
}

string webss::deserializeTuple(const Tuple& tuple)
{
	if (tuple.containerText)
	{
		string out(ASSIGN_CONTAINER_STRING);
		if (tuple.empty())
			return out + EMPTY_TUPLE;
		auto it = tuple.begin();
		return out + OPEN_TUPLE + getSeparatedValues([&]() { return ++it != tuple.end(); }, [&]() { return deserializeString(it->getString(), ConType::TUPLE); }) + CLOSE_TUPLE;
	}
	if (tuple.empty())
		return EMPTY_TUPLE;

	auto keyValues = tuple.getOrderedKeyValues();
	auto it = keyValues.begin();
	return OPEN_TUPLE + getSeparatedValues([&]() { return ++it != keyValues.end(); }, [&]() { return it->first == nullptr ? getValueOnly(*it->second, ConType::TUPLE) : getKeyValue(*it->first, *it->second, ConType::TUPLE); }) + CLOSE_TUPLE;
}

string webss::deserializeDocument(const Document& doc)
{
	if (doc.empty())
		return "";

	auto keyValues = doc.getOrderedKeyValues();
	auto it = keyValues.begin();
	return getSeparatedValues([&]() { return ++it != keyValues.end(); }, [&]() { return it->first == nullptr ? getValueOnly(*it->second, ConType::DOCUMENT) : getKeyValue(*it->first, *it->second, ConType::DOCUMENT); });
}

string webss::deserializeBlock(const Block& block)
{
	return block.getName() + deserializeWebss(block.getValue());
}