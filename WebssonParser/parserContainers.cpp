//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

const char ERROR_INPUT_DICTIONARY[] = "dictionary can only have key-values";
const char ERROR_INPUT_LIST[] = "list can only have concrete value-onlys";
const char ERROR_INPUT_TUPLE[] = "tuple can only have concrete value-onlys or key-values";
const char ERROR_INPUT_NAMESPACE[] = "namespace can only have entity definitions";
const char ERROR_INPUT_ENUM[] = "enum can only have key-onlys";
const char ERROR_INPUT_DOCUMENT[] = "document can only have concrete value-onlys or key-values";


string getItPosition(It& it)
{
	return "[ln " + to_string(it.getLine()) + ", col " + to_string(it.getCol()) + "]";
}

string getItCurrentChar(It& it)
{
	if (!it)
		return string("");

	string out;
	out += " '";
	if (*it == '\'' || *it == '\\')
		out += '\\';
	out = out + *it + "' ";

	switch (*it)
	{
	case '~': out += "(using namespace)"; break;
	case '!': out += "(entity declaration)"; break;
	case '@': out += "(import)"; break;
	case '#': out += "(option)"; break;
	case '&': out += "(self)"; break;
	case ':': out += "(line string)"; break;
	case '"': out += "(cstring)"; break;
	case '?': out += "(entity declaration)"; break;
	case '.': out += "(scope)"; break;
	case '(': case ')':out += "(round bracket / parenthesis)"; break;
	case '{': case '}': out += "(curly bracket / brace)"; break;
	case '[': case ']': out += "(square bracket)"; break;
	case '<': case '>': out += "(angle bracket / chevron)"; break;
	default:
		break;
	}
	return out;
}

Dictionary Parser::parseDictionary(It& it)
{
	static const ConType CON = ConType::DICTIONARY;
	Dictionary dict;
	if (checkEmptyContainer(it, CON))
		return dict;
	do
		if (*it == CHAR_CSTRING)
			addJsonKeyvalue(++it, dict);
		else
			parseOtherValue(it, CON,
				CaseKeyValue{ dict.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_DICTIONARY),
				ErrorValueOnly(ERROR_INPUT_DICTIONARY),
				ErrorAbstractEntity(ERROR_INPUT_DICTIONARY));
	while (checkNextElementContainer(it, CON));
	return dict;
}

List Parser::parseList(It& it)
{
	static const ConType CON = ConType::LIST;
	List list;
	if (checkEmptyContainer(it, CON))
		return list;
	do
		parseOtherValue(it, CON,
			ErrorKeyValue(ERROR_INPUT_LIST),
			ErrorKeyOnly(ERROR_INPUT_LIST),
			CaseValueOnly{ list.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_LIST));
	while (checkNextElementContainer(it, CON));
	return list;
}

Tuple Parser::parseTuple(It& it)
{
	static const ConType CON = ConType::TUPLE;
	Tuple tuple;
	if (checkEmptyContainer(it, CON))
		return tuple;
	do
		parseOtherValue(it, CON,
			CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
			ErrorKeyOnly(ERROR_INPUT_TUPLE),
			CaseValueOnly{ tuple.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_TUPLE));
	while (checkNextElementContainer(it, CON));
	return tuple;
}

List Parser::parseListText(It& it)
{
	static const ConType CON = ConType::LIST;
	List list(true);
	if (checkEmptyContainer(it, CON))
		return list;
	do
		list.add(parseLineString(it, CON));
	while (checkNextElementContainer(it, CON));
	return list;
}
Tuple Parser::parseTupleText(It& it)
{
	static const ConType CON = ConType::TUPLE;
	Tuple tuple(true);
	if (checkEmptyContainer(it, CON))
		return tuple;
	do
		tuple.add(parseLineString(it, CON));
	while (checkNextElementContainer(it, CON));
	return tuple;
}

Namespace Parser::parseNamespace(It& it, const string& name)
{
	static const ConType CON = ConType::DICTIONARY;
	Namespace nspace(name);
	if (checkEmptyContainer(it, CON))
		return nspace;
	do
		if (*it == CHAR_CONCRETE_ENTITY)
			checkMultiContainer(++it, [&]() { nspace.add(parseConcreteEntity(it)); });
		else if (*it == CHAR_ABSTRACT_ENTITY)
			checkMultiContainer(++it, [&]() { nspace.add(parseAbstractEntity(it)); });
		else
			throw runtime_error(ERROR_INPUT_NAMESPACE);
	while (checkNextElementContainer(it, CON));
	return nspace;
}
Enum Parser::parseEnum(It& it, const string& name)
{
	static const ConType CON = ConType::LIST;
	Enum tEnum(name);
	if (checkEmptyContainer(it, CON))
		return tEnum;
	do
		parseOtherValue(it, CON,
			ErrorKeyValue(ERROR_INPUT_ENUM),
			CaseKeyOnly{ tEnum.add(move(key), tEnum.size()); },
			ErrorValueOnly(ERROR_INPUT_ENUM),
			ErrorAbstractEntity(ERROR_INPUT_ENUM));
	while (checkNextElementContainer(it, CON));
	return tEnum;
}

Document Parser::parseDocument(It&& it)
{
	static const ConType CON = ConType::DOCUMENT;
#ifdef GET_LINE
	try
	{
#endif
		Document doc;
		if (checkEmptyContainer(it, CON))
			return doc;
		do
		{
			switch (*it)
			{
			case CHAR_CONCRETE_ENTITY:
				checkMultiContainer(++it, [&]() { ents.add(parseConcreteEntity(it)); });
				break;
			case CHAR_ABSTRACT_ENTITY:
				checkMultiContainer(++it, [&]() { ents.add(parseAbstractEntity(it)); });
				break;
			case CHAR_OPTION:
				checkMultiContainer(++it, [&]() { parseOption(it); });
				break;
			case CHAR_USING_NAMESPACE:
				checkMultiContainer(++it, [&]() { parseUsingNamespace(it, [&](const Entity& entity) {ents.add(entity); }); });
				break;
			case CHAR_IMPORT:
				checkMultiContainer(++it, [&]() { parseImport(it); });
				break;
			default: //parse body
				do
					parseOtherValue(it, CON,
						CaseKeyValue{ doc.addSafe(move(key), move(value)); },
						ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
						CaseValueOnly{ doc.add(move(value)); },
						ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
				while (checkNextElementContainer(it, CON));
				return doc;
			}
		} while (checkNextElementContainer(it, CON));
		return doc;
#ifdef GET_LINE
	}
	catch (exception e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
#endif
}

void Parser::checkMultiContainer(It& it, function<void()> func)
{
	static const ConType CON = ConType::DICTIONARY;
	if (*skipJunkToValid(it) != OPEN_DICTIONARY)
		func();
	else if (!checkEmptyContainer(++it, CON))
		do
			func();
		while (checkNextElementContainer(it, CON));
}

Webss Parser::parseContainerText(It& it)
{
	switch (*it)
	{
	case OPEN_DICTIONARY:
		return parseMultilineString(++it);
	case OPEN_LIST:
		return parseListText(++it);
	case OPEN_TUPLE:
		return parseTupleText(++it);
	case OPEN_FUNCTION:
		return parseFunctionText(++it);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

void Parser::parseImport(It& it)
{
	throw runtime_error("this parser cannot import documents");
}