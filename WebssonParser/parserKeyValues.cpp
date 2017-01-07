//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

pair<string, KeyType> Parser::parseKey(It& it)
{
	string key = parseName(it);
	KeyType keyType;
	if (isKeyword(key))
		keyType = KeyType::KEYWORD;
	else if (vars.hasVariable(key))
	{
		if (!skipWhitespace(it))
			keyType = KeyType::VARIABLE;
		else if (*it != '\n')
			keyType = *it == CHAR_SCOPE ? KeyType::SCOPE : KeyType::VARIABLE;
		else if (!skipJunk(++it) || *it != CHAR_SCOPE)
		{
			lineGreed = true;
			keyType = KeyType::VARIABLE;
		}
		else
			keyType = KeyType::SCOPE;
	}
	else if (varsBlockId.hasVariable(key))
		keyType = KeyType::BLOCK_VALUE;
	else
	{
		if (!skipWhitespace(it))
			keyType = KeyType::KEYNAME;
		else if (*it != '\n')
		{
			if (isKeyChar(*it))
			{
				if (*it == CHAR_SCOPE)
					throw runtime_error(ERROR_DEREFERENCED_VARIABLE);
				else
					keyType = getKeyType(*it);
			}
			else
				keyType = KeyType::KEYNAME;
		}
		else if (!skipJunk(++it) || !isKeyChar(*it) || *it == CHAR_SCOPE)
		{
			lineGreed = true;
			keyType = KeyType::KEYNAME;
		}
		else
			keyType = getKeyType(*it);
	}

	return{ move(key), keyType };
}

//DONE
Webss Parser::parseValue(It& it, ConType con)
{
	switch (*it)
	{
	case OPEN_DICTIONARY:
		return parseDictionary(++it);
	case OPEN_LIST:
		return parseList(++it);
	case OPEN_TUPLE:
		return parseTuple(++it);
	case OPEN_FUNCTION:
		return parseFunction(++it);
	case CHAR_COLON:
		return parseValueColon(++it, con);
	case CHAR_EQUAL:
		return parseValueEqual(++it, con);
	case CHAR_CSTRING:
		return parseCString(++it);
	default:
		throw domain_error(webss_ERROR_UNEXPECTED);
	}
}

//DONE
void Parser::addJsonKeyvalue(It& it, Dictionary& dict)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });

	auto keyPair = parseKey(it);
	if (keyPair.second != KeyType::CSTRING)
		throw runtime_error("invalid key name in supposed Json key-value");

	skipJunkToValidCondition(++it, [&]() { return *it == CHAR_COLON; });
	dict.addSafe(move(keyPair.first), parseValueEqual(++it, ConType::DICTIONARY));
}

//DONE
Webss Parser::parseValueColon(It& it, ConType con)
{
	if (!it)
		return string("");
	else if (*it != CHAR_COLON)
		return parseLineString(it, con);
	else
		return parseContainerString(skipJunkToValid(++it));
}

//DONE
Webss Parser::parseValueEqual(It& it, ConType con)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_CSTRING:
		return parseValue(it, con);
	default:
		if (isNumberStart(*it))
			return parseNumber(it);
		else if (isNameStart(*it))
			return parseValueEqualNameStart(it, con);
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}

Webss Parser::parseValueEqualNameStart(It& it, ConType con)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case KeyType::KEYWORD:
		return Keyword(keyPair.first);
	case KeyType::KEYNAME:
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(keyPair.first));
	case KeyType::VARIABLE:
		return checkIsValue(vars[keyPair.first]);
	case KeyType::SCOPE:
		return checkIsValue(parseScopedValue(it, keyPair.first));
	case KeyType::BLOCK_VALUE:
		return parseBlockValue(it, con, keyPair.first);
	default:
		throw runtime_error("assigned named value must be a keyword or a variable");
	}
}

//TODO: if last scoped item is a block, get block value
//*it must point to scope char
const Variable& Parser::parseScopedValue(It& it, const string& varName)
{
	try
	{
		Variable* var = &vars[varName];
		pair<string, KeyType> keyPair;
		do
		{
			skipJunkToValidCondition(++it, [&]() { return isNameStart(*it); });
			keyPair = parseKey(it);
			var = const_cast<Variable*>(&var->getContent().getNamespace().at(keyPair.first));
		} while (keyPair.second == KeyType::SCOPE);
		return *var;
	}
	catch (exception e)
	{
		throw runtime_error("could not get scoped value");
	}
}