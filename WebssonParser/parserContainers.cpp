//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

string getItPosition(It& it)
{
	return "[ln " + to_string(it.getLine()) + ", ch " + to_string(it.getCharCol()) + "]";
}

string getItCurrentChar(It& it)
{
	if (!it)
		return string("");

	string out;
	out += '\'';
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
	case '|': out += "(name alias)"; break;
	case ':': out += "(line string)"; break;
	case '"': out += "(cstring)"; break;
	case '?': out += "(variable declaration)"; break;
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

#define PatternOtherValuesDictionary(Name, Con) \
auto other = parseOtherValue(it, Con); \
switch (other.type) \
{ \
case OtherValue::Type::KEY_VALUE: \
	Name.addSafe(move(other.key), move(other.value)); \
	break; \
default: \
	throw runtime_error("unexpected value"); \
}

#define PatternOtherValuesList(Name, Con) \
auto other = parseOtherValue(it, Con); \
switch (other.type) \
{ \
case OtherValue::Type::VALUE_ONLY: \
	Name.add(move(other.value)); \
	break; \
default: \
	throw runtime_error("unexpected value"); \
}

#define PatternOtherValuesTuple(Name, Con) \
auto other = parseOtherValue(it, Con); \
switch (other.type) \
{ \
case OtherValue::Type::KEY_VALUE: \
	Name.addSafe(move(other.key), move(other.value)); \
	break; \
case OtherValue::Type::VALUE_ONLY: \
	Name.add(move(other.value)); \
	break; \
default: \
	throw runtime_error("unexpected value"); \
}

#define PatternOtherValuesEnum(Name, Con) \
auto other = parseOtherValue(it, Con); \
switch (other.type) \
{ \
case OtherValue::Type::KEY_ONLY: \
	Name.addSafe(move(other.key), Name.size()); \
	break; \
default: \
	throw runtime_error("unexpected value"); \
}

Dictionary Parser::parseDictionary(It& it)
{
	PatternParse(Dictionary cont; CheckEmpty(ConType::DICTIONARY, return cont), cont, ConType::DICTIONARY, CLOSE_DICTIONARY, ExtraCase(CHAR_CSTRING, addJsonKeyvalue(++it, cont)), PatternOtherValuesDictionary(cont, ConType::DICTIONARY))
}
List Parser::parseList(It& it)
{
	PatternParse(List cont; CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, ExtraCase(webss_CHAR_ANY_CONTAINER_CHAR_VALUE, cont.add(parseCharValue(it, ConType::LIST))), PatternOtherValuesList(cont, ConType::LIST))
}
Tuple Parser::parseTuple(It& it)
{
	PatternParse(Tuple cont; CheckEmpty(ConType::TUPLE, return cont), cont, ConType::TUPLE, CLOSE_TUPLE, ExtraCase(webss_CHAR_ANY_CONTAINER_CHAR_VALUE, cont.add(parseCharValue(it, ConType::TUPLE))), PatternOtherValuesTuple(cont, ConType::TUPLE))
}

List Parser::parseListText(It& it)
{
	PatternParse(List cont(true); CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, , cont.add(parseLineString(it, ConType::LIST));)
}
Tuple Parser::parseTupleText(It& it)
{
	PatternParse(Tuple cont(true); CheckEmpty(ConType::TUPLE, return cont), cont, ConType::TUPLE, CLOSE_TUPLE, , cont.add(parseLineString(it, ConType::TUPLE));)
}

#define CaseNamespace(Char, Func) ExtraCase(Char, checkMultiContainer(++it, [&]() { cont.add(Func); }))
#define ExtraCasesNamespace \
	CaseNamespace(CHAR_CONCRETE_ENTITY, parseConcreteEntity(it)) \
	CaseNamespace(CHAR_ABSTRACT_ENTITY, parseAbstractEntity(it))

Namespace Parser::parseNamespace(It& it, const string& name)
{
	PatternParse(Namespace cont(name); CheckEmpty(ConType::DICTIONARY, return cont), cont, ConType::DICTIONARY, CLOSE_DICTIONARY, ExtraCasesNamespace, throw runtime_error(ERROR_KEY_NAMESPACE);)
}
Enum Parser::parseEnum(It& it, const string& name)
{
	PatternParse(Enum cont(name); CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, , PatternOtherValuesEnum(cont, ConType::LIST))
}

#define CON ConType::DOCUMENT
Document Parser::parseDocument(It& it)
{
#ifdef GET_LINE
	try
	{
#endif
		Document doc;

	documentHead:
		if (!skipJunk(it))
			return doc;

		switch (*it)
		{
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { parseConcreteEntity(it, [&](const Variable& var) { vars.add(var); }); });
			break;
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { vars.add(parseAbstractEntity(it)); });
			break;
		case CHAR_OPTION:
			checkMultiContainer(++it, [&]() { parseOption(it); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]() { parseUsingNamespace(it); });
			break;
		case CHAR_IMPORT:
#ifdef ALLOW_IMPORT
			checkMultiContainer(++it, [&]() { parseImport(it); });
			break;
#else
			throw runtime_error("this parser cannot import documents");
#endif
		default:
			goto documentBody;
		}
		checkToNextElement(it, CON);
		goto documentHead;

	documentBody:
		do
		{
			switch (*it)
			{
			case webss_CHAR_ANY_CONTAINER_CHAR_VALUE:
				doc.add(parseCharValue(it, CON));
				break;
			default:
				if (checkSeparator(it))
					continue;
				PatternOtherValuesTuple(doc, ConType::DOCUMENT)
			}
			checkToNextElement(it, CON);
		} while (it);
		return doc;
#ifdef GET_LINE
	}
	catch (exception e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
#endif
}
#undef CON

void Parser::checkMultiContainer(It& it, function<void()> func)
{
	if (*skipJunkToValid(it) != OPEN_DICTIONARY)
		func();
	else
	{
		++it;
		PatternParse(CheckEmpty(ConType::DICTIONARY, return), , ConType::DICTIONARY, CLOSE_DICTIONARY, , func();)
	}
}

Webss Parser::parseContainerText(It& it)
{
	switch (*it)
	{
	case OPEN_DICTIONARY:
		return parseDictionaryText(++it);
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