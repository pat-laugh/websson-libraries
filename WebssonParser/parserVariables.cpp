//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

Variable Parser::parseVariable(It& it)
{
	auto name = parseNameSafe(it);
	skipJunkToValidCondition(it, [&]() { return isKeyChar(*it); });
	return Variable(move(name), parseValue(it, ConType::DOCUMENT));
}

const char ERROR_BLOCK_ID[] = "a block must have an integer id";
Variable Parser::parseBlock(It& it)
{
	auto name = parseNameSafe(it);
	Variable var;
	switch (*skipJunkToValid(it))
	{
	case OPEN_TUPLE: //block id
		try
		{
			skipJunkToValid(++it);
			type_int value;
			if (isNumberStart(*it))
				value = parseNumber(it).getInt();
			else if (isNameStart(*it))
				value = vars.at(parseName(it)).getContent().getInt();
			else
				throw;

			BlockId id(name, value);
			new (&var) Variable(move(name), move(id));
		}
		catch (exception e)
		{
			throw runtime_error(ERROR_BLOCK_ID);
		}
		skipJunkToValidCondition(it, [&]() { return *it == CLOSE_TUPLE; });
		++it;
		break;
	case OPEN_DICTIONARY: //namespace
	{
		auto nspace = parseNamespace(++it, name);
		new (&var) Variable(move(name), move(nspace));
		break;
	}
	case OPEN_LIST: //enum
	{
		auto nspace = parseEnum(++it, name);
		new (&var) Variable(move(name), Webss(move(nspace), true));
		break;
	}
	case OPEN_FUNCTION: //fhead
	{
		using Type = FunctionHeadSwitch::Type;
		auto headSwitch = parseFunctionHead(++it);
		switch (headSwitch.t)
		{
		case Type::STANDARD:
			new (&var) Variable(move(name), move(headSwitch.fheadStandard));
			break;
		case Type::BINARY:
			new (&var) Variable(move(name), move(headSwitch.fheadBinary));
			break;
		default:
			throw logic_error("");
		}
		break;
	}
	case CHAR_COLON: //text fhead
	{
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		new (&var) Variable(move(name), parseFunctionHeadText(++it));
		break;
	}
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
	return var;
}

//DONE
string Parser::parseName(It& it)
{
	string name;
	do
		name += *it;
	while (++it && isNameBody(*it));
	return name;
}

//DONE
string Parser::parseNameSafe(It& it)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	string name = parseName(it);
	if (nameExists(name))
		throw runtime_error(webss_ERROR_VARIABLE_EXISTS(name));
	return name;
}

string Parser::parseVariableString(It& it)
{
	string varName = parseName(it);
	if (!vars.hasVariable(varName))
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(varName));

	const Webss* value = &vars[varName].getContent();
	do
	{
		if (it != CHAR_SCOPE)
		{
			if (it == CHAR_COLON && it.peekGood() && isNameBody(it.peek()))
				++it;
			if (!value->isString())
				throw runtime_error(ERROR_VARIABLE_STRING);
			
			return value->getString();
		}

		if (!(++it) || !isNameStart(*it))
		{
			if (it == CHAR_COLON && it.peekGood() && isNameBody(it.peek()))
				++it;
			if (!value->isString())
				throw runtime_error(ERROR_VARIABLE_STRING);
			return value->getString() + CHAR_SCOPE;
		}
		if (!value->isDictionary())
			throw runtime_error(ERROR_DEREFERENCED_VARIABLE);
		value = &value->getDictionary().at(parseName(it));
	} while (true);
}

bool Parser::nameExists(const string& name)
{
	return isKeyword(name) || vars.hasVariable(name) || varsBlockId.hasVariable(name);
}