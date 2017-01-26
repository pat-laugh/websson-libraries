//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"
#ifdef webss_ALLOW_IMPORT
#include "curl.h"
#endif

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
	case '(': case ')':out += "(parenthesis / round bracket)"; break;
	case '{': case '}': out += "(brace / curly bracket)"; break;
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
		list.add(parseValueOnly(it, CON));
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

Namespace Parser::parseNamespace(It& it, const string& name, const Namespace& previousNamespace)
{
	static const ConType CON = ConType::DICTIONARY;
	auto& nspace = Namespace::make(name, previousNamespace);
	if (checkEmptyContainer(it, CON))
		return nspace;
	do
		switch (*it)
		{
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { nspace.addSafe(parseConcreteEntity(it, CON)); });
			break;
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { nspace.addSafe(parseAbstractEntity(it, nspace)); });
			break;
		case CHAR_SELF:
			skipJunkToValidCondition(++it, [&]() { return *it == OPEN_FUNCTION; });
			nspace.addSafe(Entity(string(name), parseFunctionHead(++it)));
			break;
		default:
			throw runtime_error(ERROR_INPUT_NAMESPACE);
		}
	while (checkNextElementContainer(it, CON));
	return nspace;
}
Enum Parser::parseEnum(It& it, const string& name)
{
	static const ConType CON = ConType::LIST;
	auto&& tEnum = Enum::make(name);
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
		if (!checkEmptyContainer(it, CON) && !parseDocumentHead(it, doc.getHead(), CON, Namespace::getEmptyInstance()))
		{
			ParamDocumentIncluder includer(doc.getHead());
			do
				parseOtherValue(it, CON,
					CaseKeyValue{ doc.addSafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
					CaseValueOnly{ doc.add(move(value)); },
					ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
			while (checkNextElementContainer(it, CON));
		}
		return doc;
#ifdef GET_LINE
	}
	catch (exception e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
#endif
}

bool Parser::parseDocumentHead(It& it, vector<ParamDocument>& docHead, ConType con, const Namespace& nspace)
{
	do
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseAbstractEntity(it, nspace); docHead.push_back(ParamDocument(ent)); ents.addLocalSafe(move(ent)); });
			break;
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseConcreteEntity(it, con); docHead.push_back(ParamDocument(ent)); ents.addLocalSafe(move(ent)); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]()
			{
				skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
				auto nameType = parseNameType(it);
				if (nameType.type != NameType::ENTITY || !nameType.entity.getContent().isNamespace())
					throw runtime_error("expected namespace");
				docHead.push_back(ParamDocument(nameType.entity.getContent().getNamespace()));
			});
			break;
		case CHAR_SCOPED_DOCUMENT:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseScopedDocument(it)); });
			break;
		case CHAR_IMPORT:
			checkMultiContainer(++it, [&]() { docHead.push_back(parseImport(it, con)); });
			break;
		default:
			return false;
		}
	while (checkNextElementContainer(it, con));
	return true;
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

ScopedDocument Parser::parseScopedDocument(It& it)
{
	static const ConType CON = ConType::DICTIONARY;
	static const auto& currentNamespace = Namespace::getEmptyInstance();
	if (*it != OPEN_FUNCTION)
		throw runtime_error(ERROR_UNEXPECTED);
	auto head = parseFunctionHeadScoped(++it);

	skipJunkToValidCondition(it, [&]() { return *it == OPEN_DICTIONARY; });
	DocumentHead body;
	if (!checkEmptyContainer(++it, CON))
	{
		ParamDocumentIncluder includer(static_cast<vector<ParamDocument>>(head.getParameters()));
		if (!parseDocumentHead(it, body, CON, Namespace::getEmptyInstance()))
			throw runtime_error(ERROR_UNEXPECTED);
	}
	return{ move(head), move(body) };
}

ImportedDocument Parser::parseImport(It& it, ConType con)
{
#ifndef webss_ALLOW_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	static const ConType CON = ConType::DOCUMENT;
	try
	{
		ImportedDocument import{ parseValueOnly(it, con) };
		const auto& link = import.getLink();
		if (importedDocuments.hasEntity(link))
			import.content = importedDocuments[link];
		else
		{
			It itImported(Curl().readWebDocument(link));
			DocumentHead docHead;
			if (!checkEmptyContainer(itImported, CON) && !parseDocumentHead(itImported, docHead, CON, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
			auto ent = BasicEntity<DocumentHead>(link, move(docHead));
			import.content = ent;
			importedDocuments.addLocal(move(ent));
		}
		return import;
	}
	catch (exception e)
	{
		throw runtime_error(string("while parsing import, ") + e.what());
	}
#endif
}