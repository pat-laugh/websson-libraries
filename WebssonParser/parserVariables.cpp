//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

void Parser::parseConcreteEntity(It& it, function<void(const Variable& var)> funcForEach)
{
	Variable var;
	parseOtherValue(it, ConType::DOCUMENT,
		CaseKeyValue{ new (&var) Variable(move(key), move(value)); funcForEach(var); },
		CaseKeyOnly{ throw runtime_error(ERROR_EXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity{ throw runtime_error(webss_ERROR_VARIABLE_EXISTS(abstractEntity.getName())); },
		CaseAlias{ Variable alias(move(key), Webss(var)); funcForEach(alias); });
}

Variable Parser::parseAbstractEntity(It& it)
{
	auto name = parseNameSafe(it);
	switch (*skipJunkToValid(it))
	{
	case CHAR_BLOCK:
		return Variable(move(name), Webss(parseBlockHead(++it), true));
	case OPEN_DICTIONARY:
		return Variable(move(name), parseNamespace(++it, name));
	case OPEN_LIST:
		return Variable(move(name), Webss(parseEnum(++it, name), true));
	case OPEN_FUNCTION:
	{
		using Type = FunctionHeadSwitch::Type;
		auto headSwitch = parseFunctionHead(++it);
		switch (headSwitch.t)
		{
		case Type::STANDARD:
			return Variable(move(name), move(headSwitch.fheadStandard));
		case Type::BINARY:
			return Variable(move(name), move(headSwitch.fheadBinary));
		default:
			throw logic_error("");
		}
	}
	case CHAR_COLON:
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		return Variable(move(name), parseFunctionHeadText(++it));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string Parser::parseName(It& it)
{
	string name;
	do
		name += *it;
	while (++it && isNameBody(*it));
	return name;
}

string Parser::parseNameSafe(It& it)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	string name = parseName(it);
	if (nameExists(name))
		throw runtime_error(webss_ERROR_VARIABLE_EXISTS(name));
	return name;
}

bool Parser::nameExists(const string& name)
{
	return isKeyword(name) || vars.hasVariable(name) || varsBlockId.hasVariable(name);
}

void Parser::parseUsingNamespace(It& it, function<void(const Variable& var)> funcForEach)
{
	if (*skipJunkToValid(it) == OPEN_FUNCTION)
		//....
		return;

	parseOtherValue(it, ConType::DOCUMENT,
		CaseKeyValue{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseKeyOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity
		{
			for (const auto& var : checkIsNamespace(abstractEntity))
				funcForEach(var);
		},
		CaseAlias{ throw runtime_error(ERROR_UNEXPECTED); });
}