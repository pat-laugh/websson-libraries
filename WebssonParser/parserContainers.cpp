//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

Tuple Parser::parseTuple(It& it)
{
	return parseContainer<CLOSE_TUPLE, ConType::TUPLE>(it);
}

void Parser::parseContainerNameStart(It& it, Tuple& tuple, ConType con)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
		tuple.addSafe(move(keyPair.first), parseValue(it, con));
		break;
	case KeyType::KEYWORD:
		tuple.add(Keyword(keyPair.first));
		break;
	case KeyType::KEYNAME:
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(keyPair.first));
	case KeyType::VARIABLE:
		tuple.add(checkIsValue(vars[keyPair.first]));
		break;
	case KeyType::SCOPE:
		tuple.add(checkIsValue(parseScopedValue(it, keyPair.first)));
		break;
	case KeyType::BLOCK_VALUE:
		tuple.add(parseBlockValue(it, con, keyPair.first));
		break;
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}

#define CON ConType::LIST
List Parser::parseList(It& it)
{
	if (checkEmptyContainer(it, CON))
		return List();

	List list;
	do
	{
		switch (*it)
		{
		case CLOSE_LIST:
			checkContainerEnd(it);
			return list;
		case webss_CHAR_ANY_CONTAINER_CHAR_VALUE:
			list.add(parseValue(it, CON));
			break;
		default:
			if (checkOtherValues(it, [&]() { parseListNameStart(it, list); }, [&]() { list.add(parseNumber(it)); }))
				continue;
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_CONTAINER_NOT_CLOSED);
}

void Parser::parseListNameStart(It& it, List& list)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
		throw runtime_error(ERROR_ADD_KEY_LIST);
	case KeyType::KEYWORD:
		list.add(Keyword(keyPair.first));
		break;
	case KeyType::KEYNAME:
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(keyPair.first));
	case KeyType::VARIABLE:
		list.add(checkIsValue(vars[keyPair.first]));
		break;
	case KeyType::SCOPE:
		list.add(checkIsValue(parseScopedValue(it, keyPair.first)));
		break;
	case KeyType::BLOCK_VALUE:
		list.add(parseBlockValue(it, CON, keyPair.first));
		break;
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}
#undef CON

Dictionary Parser::parseDictionary(It& it)
{
#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)
#define CON ConType::DICTIONARY
	if (checkEmptyContainer(it, CON))
		return Dictionary();

	Dictionary dict;
	do
	{
		switch (*it)
		{
		case CLOSE_DICTIONARY:
			checkContainerEnd(it);
			return dict;
		case CHAR_CSTRING:
			addJsonKeyvalue(++it, dict);
			break;
		default:
			if (checkSeparator(it))
				continue;
			if (!isNameStart(*it))
				THROW_ERROR;

			auto keyPair = parseKey(it);
			switch (keyPair.second)
			{
			case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
				dict.addSafe(move(keyPair.first), parseValue(it, CON));
				break;
			case KeyType::KEYWORD: case KeyType::KEYNAME: case KeyType::VARIABLE: case KeyType::SCOPE: case KeyType::BLOCK_VALUE:
				THROW_ERROR;
			default:
				throw runtime_error(webss_ERROR_UNEXPECTED);
			}
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_CONTAINER_NOT_CLOSED);
#undef CON
#undef THROW_ERROR
}

Namespace Parser::parseNamespaceContainer(It& it, const string& name)
{
#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)
#define CON ConType::DICTIONARY
	Namespace nspace(name);
	if (checkEmptyContainer(it, CON))
		return nspace;

	do
	{
		switch (*it)
		{
		case CLOSE_DICTIONARY:
			checkContainerEnd(it);
			return nspace;
		case CHAR_VARIABLE:
			checkMultiContainer(++it, [&]() { nspace.add(parseVariable(it)); });
			break;
		case CHAR_BLOCK:
			checkMultiContainer(++it, [&]() { nspace.add(parseBlock(it)); });
			break;
		default:
			if (checkSeparator(it))
				continue;
			throw runtime_error(webss_ERROR_UNEXPECTED);
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_CONTAINER_NOT_CLOSED);
#undef CON
#undef THROW_ERROR
}

Namespace Parser::parseEnum(It& it, const string& name)
{
#define THROW_ERROR throw runtime_error(ERROR_ANONYMOUS_KEY)
#define CON ConType::LIST
	Namespace nspace(move(name));
	if (checkEmptyContainer(it, CON))
		return nspace;

	do
	{
		switch (*it)
		{
		case CLOSE_LIST:
			checkContainerEnd(it);
			return nspace;
		default:
			if (checkSeparator(it))
				continue;
			if (!isNameStart(*it))
				THROW_ERROR;

			auto keyPair = parseKey(it);
			switch (keyPair.second)
			{
			case KeyType::KEYNAME:
				nspace.addSafe(move(keyPair.first), nspace.size());
				break;
			case KeyType::KEYWORD: case KeyType::VARIABLE: case KeyType::SCOPE: case KeyType::BLOCK_VALUE:
				THROW_ERROR;
			default:
				throw runtime_error(webss_ERROR_UNEXPECTED);
			}
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_CONTAINER_NOT_CLOSED);
#undef CON
#undef THROW_ERROR
}

Tuple Parser::parseTupleText(It& it)
{
	return Tuple(parseContainerText<CLOSE_TUPLE, ConType::TUPLE>(it), true);
}

List Parser::parseListText(It& it)
{
	return List(parseContainerText<CLOSE_LIST, ConType::LIST>(it), true);
}

string Parser::parseDictionaryText(It& it)
{
	if (*skipJunkToValid(it) == CLOSE_DICTIONARY)
		return "";

	string text;
	int countStartEnd = 1; //count of dictionary start - dictionary end
	bool addSpace = false; //== false if last char was \e or \s
loopStart:
	text += parseLineStringTextDictionary(it, countStartEnd, addSpace); //function throws errors if it is end or dictionary not closed
	if (countStartEnd == 0)
	{
		++it;
		return text;
	}
	if (addSpace)
		text += ' ';
	goto loopStart;
}

Block Parser::parseBlockValue(It& it, ConType con, const string& blockName)
{
	return Block(varsBlockId[blockName], checkIsValue(parseValueEqual(it, con)));
}

Block Parser::parseBlockValue(It& it, ConType con, const BasicVariable<BlockId>& blockId)
{
	return Block(blockId, checkIsValue(parseValueEqual(it, con)));
}