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

void Parser::addJsonKeyvalue(It& it, Dictionary& dict)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });

	auto keyPair = parseKey(it);
	if (keyPair.second != KeyType::CSTRING)
		throw runtime_error("invalid key name in supposed Json key-value");

	skipJunkToValidCondition(++it, [&]() { return *it == CHAR_COLON; });
	dict.addSafe(move(keyPair.first), parseValueEqual(++it, ConType::DICTIONARY));
}

Webss Parser::parseValueColon(It& it, ConType con)
{
	if (!it)
		return string("");
	else if (*it != CHAR_COLON)
		return parseLineString(it, con);
	else
		return parseContainerText(skipJunkToValid(++it));
}

Webss Parser::parseValueEqual(It& it, ConType con)
{
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_CSTRING:
		return parseCharValue(it, con);
	default:
		if (isNumberStart(*it))
			return parseNumber(it);
		else if (isNameStart(*it))
			return parseValueEqualNameStart(it, con);
		throw runtime_error(ERROR_UNEXPECTED);
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
		return checkIsConcrete(vars[keyPair.first]);
	case KeyType::SCOPE:
		return checkIsConcrete(parseScopedValue(it, keyPair.first));
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

Parser::OtherValue Parser::parseOtherValue(It& it, ConType con)
{
	using Type = OtherValue::Type;
	if (isNameStart(*it))
	{
		auto keyPair = parseKey(it);
		switch (keyPair.second)
		{
		case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
			return{ Type::KEY_VALUE, move(keyPair.first), parseCharValue(it, con) };
		case KeyType::KEYWORD:
			return{ Type::VALUE_ONLY, string(), Keyword(keyPair.first) };
		case KeyType::KEYNAME:
			return{ Type::KEY_ONLY, move(keyPair.first) };
		case KeyType::VARIABLE:
			return checkOtherValueVariable(it, con, vars[keyPair.first]);
		case KeyType::SCOPE:
			return checkOtherValueVariable(it, con, parseScopedValue(it, keyPair.first));
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}
	else if (isNumberStart(*it))
		return{ Type::VALUE_ONLY, string(), parseNumber(it) };
	else
		throw runtime_error(ERROR_UNEXPECTED);
}

Parser::OtherValue Parser::checkOtherValueVariable(It& it, ConType con, const Variable& var)
{
	using Type = OtherValue::Type;
	const auto& content = var.getContent();
	if (content.isConcrete())
		return{ Type::VALUE_ONLY, string(), var };
	
	switch (content.getType())
	{
	case WebssType::BLOCK_HEAD:
	case WebssType::FUNCTION_HEAD_BINARY:
		PatternLineGreed(*it == OPEN_TUPLE, return{ Type::VALUE_ONLY, string(), parseFunctionBodyBinary(it, content.getFunctionHeadBinary().getParameters()) }, break)
	case WebssType::FUNCTION_HEAD_MANDATORY:
		//...
	case WebssType::FUNCTION_HEAD_SCOPED:
		//...
	case WebssType::FUNCTION_HEAD_STANDARD:
		PatternLineGreed(*it == OPEN_TUPLE, return{ Type::VALUE_ONLY, string(), parseFunctionBodyStandard(it, content.getFunctionHeadStandard().getParameters()) }, break)
	default:
		break;
	}
	return{ Type::ABSTRACT_ENTITY, string(), Webss(), var };
}