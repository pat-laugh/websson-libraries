//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

#define PatternLineGreed(ConditionSuccess, Success, Failure) { \
if (!skipWhitespace(it)) \
	{ Failure; } \
else if (*it != '\n') \
{ \
	if (ConditionSuccess) \
		{ Success; } \
	else \
		{ Failure; } \
} \
else if (!skipJunk(++it) || !(ConditionSuccess)) \
{ \
	lineGreed = true; \
	Failure; \
} \
else \
	{ Success; } }

//DONE
pair<string, KeyType> Parser::parseKey(It& it)
{
	string key = parseName(it);
	KeyType keyType;
	if (isKeyword(key))
		keyType = KeyType::KEYWORD;
	else if (vars.hasVariable(key))
		PatternLineGreed(*it == CHAR_SCOPE, keyType = KeyType::SCOPE, keyType = KeyType::VARIABLE)
	else
		PatternLineGreed(isKeyChar(*it), keyType = getKeyType(*it), keyType = KeyType::KEYNAME)

	return{ move(key), keyType };
}

//DONE
Webss Parser::parseCharValue(It& it, ConType con)
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
		throw domain_error(ERROR_UNEXPECTED);
	}
}

//DONE
void Parser::addJsonKeyvalue(It& it, Dictionary& dict)
{
	auto keyPair = parseKey(skipJunkToValidCondition(it, [&]() { return isNameStart(*it); }));
	if (keyPair.second != KeyType::CSTRING)
		throw runtime_error("invalid key name in supposed Json key-value");

	skipJunkToValidCondition(++it, [&]() { return *it == CHAR_COLON; });
	dict.addSafe(move(keyPair.first), parseValueEqual(++it, ConType::DICTIONARY));
}

//DONE
Webss Parser::parseValueColon(It& it, ConType con)
{
	return it != CHAR_COLON ? parseLineString(it, con) : parseContainerText(skipJunkToValid(++it));
}

//DONE
Webss Parser::parseValueEqual(It& it, ConType con)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_CSTRING:
		return parseCharValue(it, con);
	default:
		auto other = parseOtherValue(it, con);
		if (other.type == OtherValue::Type::VALUE_ONLY)
			return move(other.value);
	}
	throw runtime_error(ERROR_VALUE);
}

//DONE
const Variable& Parser::parseScopedValue(It& it, const string& varName) //used to be at line 121 now at line 98!
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

//DONE
Parser::OtherValue Parser::parseOtherValue(It& it, ConType con)
{
	using Type = OtherValue::Type;
	if (isNameStart(*it))
	{
		auto keyPair = parseKey(it);
		switch (keyPair.second)
		{
		case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
			return{ move(keyPair.first), parseCharValue(it, con) };
		case KeyType::KEYWORD:
			return{ Keyword(keyPair.first) };
		case KeyType::KEYNAME:
			return{ move(keyPair.first) };
		case KeyType::VARIABLE:
			return checkOtherValueVariable(it, con, vars[keyPair.first]);
		case KeyType::SCOPE:
			return checkOtherValueVariable(it, con, parseScopedValue(it, keyPair.first));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}
	else if (isNumberStart(*it))
		return{ parseNumber(it) };
	else
		throw runtime_error(ERROR_UNEXPECTED);
}

Parser::OtherValue Parser::checkOtherValueVariable(It& it, ConType con, const Variable& var)
{
	using Type = OtherValue::Type;
	const auto& content = var.getContent();
	if (content.isConcrete())
		return{ Webss(var) };
	
	switch (content.getType())
	{
	case WebssType::BLOCK_HEAD:
	case WebssType::FUNCTION_HEAD_BINARY:
		PatternLineGreed(*it == OPEN_TUPLE, return{ parseFunctionBodyBinary(it, content.getFunctionHeadBinary().getParameters()) }, break)
	case WebssType::FUNCTION_HEAD_MANDATORY:
		//...
	case WebssType::FUNCTION_HEAD_SCOPED:
		//...
	case WebssType::FUNCTION_HEAD_STANDARD:
		PatternLineGreed(*it == OPEN_TUPLE, return{ parseFunctionBodyStandard(it, content.getFunctionHeadStandard().getParameters()) }, break)
	default:
		break;
	}
	return{ var };
}

void Parser::parseOtherValue(It& it, ConType con, std::function<void(string&& key, Webss&& value)> funcKeyValue, function<void(string&& key)> funcKeyOnly, function<void(Webss&& value)> funcValueOnly, function<void(const Variable& abstractEntity)> funcAbstractEntity, function<void (string&& key)> funcAlias)
{
	auto other = parseOtherValue(it, con);
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		funcKeyValue(move(other.key), move(other.value));
		break;
	case OtherValue::Type::KEY_ONLY:
		funcKeyOnly(move(other.key));
		break;
	case OtherValue::Type::VALUE_ONLY:
		funcValueOnly(move(other.value));
		break;
	case OtherValue::Type::ABSTRACT_ENTITY:
		funcAbstractEntity(other.abstractEntity);
		break;
	default:
		auto it = other.aliases.begin();
		(other.type == OtherValue::Type::KEY_VALUE_ALIASES ? funcKeyValue(move(*it), move(other.value)) : funcKeyOnly(move(*it)));
		while (++it != other.aliases.end())
			funcAlias(move(*it));
		break;
	}
}