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
	static const auto& currentNamespace = Namespace::getEmptyInstance();
#ifdef GET_LINE
	try
	{
#endif
		Document doc;
		auto& docHead = doc.getHead();
		if (checkEmptyContainer(it, CON))
			return doc;
		do
		{
			switch (*it)
			{
			case CHAR_ABSTRACT_ENTITY:
				checkMultiContainer(++it, [&]() { auto ent = parseAbstractEntity(it, currentNamespace); docHead.push_back(ParamDocument(ent)); ents.addLocal(move(ent)); });
				break;
			case CHAR_CONCRETE_ENTITY:
				checkMultiContainer(++it, [&]() { auto ent = parseConcreteEntity(it, CON); docHead.push_back(ParamDocument(ent, true)); ents.addLocal(move(ent)); });
				break;
			case CHAR_USING_NAMESPACE:
				checkMultiContainer(++it, [&]() { docHead.push_back(parseScopedDocument(it)); });
				break;
			case CHAR_IMPORT:
				checkMultiContainer(++it, [&]() { docHead.push_back(parseImport(it, CON)); });
				break;
			case CHAR_OPTION:
				checkMultiContainer(++it, [&]() { parseOption(it); });
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
		{
			if (!isNameStart(*it))
				throw runtime_error(ERROR_UNEXPECTED);
			func();
		}
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
	if (checkEmptyContainer(++it, CON))
		return{ move(head), move(body) };

	vector<Entity> entitiesToReAdd;

	//get ents
	const auto& params = head.getParameters();
	for (const auto& param : params)
		if (param.hasEntity())
			ents.addLocalSafe(param.getEntity());
		else
		{
			const auto& nspace = param.getNamespace();
			const auto& name = nspace.getName();
			if (ents.hasEntity(name))
			{
				const auto& ent = ents.getWebss(name);
				const auto& content = ent.getContent();
				if (content.isNamespace() && content.getNamespace().getPointer() == nspace.getPointer())
				{
					entitiesToReAdd.push_back(ent);
					ents.removeLocal(name);
				}
			}
			for (const auto& ent : nspace)
				ents.addLocalSafe(ent);
		}

	do
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseAbstractEntity(it, currentNamespace); body.push_back(ParamDocument(ent)); ents.addLocal(move(ent)); });
			break;
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseConcreteEntity(it, CON); body.push_back(ParamDocument(ent, true)); ents.addLocal(move(ent)); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]() { body.push_back(parseScopedDocument(it)); });
			break;
		case CHAR_IMPORT:
			checkMultiContainer(++it, [&]() { body.push_back(parseImport(it, CON)); });
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	} while (checkNextElementContainer(it, CON));

	//remove ents
	for (const auto& param : params)
		if (param.hasEntity())
			ents.removeLocal(param.getEntity());
		else
			for (const auto& ent : param.getNamespace())
				ents.removeLocal(ent);

	for (const auto& ent : entitiesToReAdd)
		ents.addLocal(ent);

	return{ move(head), move(body) };
}

ImportedDocument Parser::parseImport(It& it, ConType con)
{
#ifndef webss_ALLOW_IMPORT
	throw runtime_error("this parser cannot import documents");
#else

	ImportedDocument import;
	auto& name = import.name;
	auto& body = import.body;

	try
	{
		name = parseValueOnly(it, con);
		const auto& link = name.getString();
		if (importedDocuments.hasEntity(link))
			body = importedDocuments[link].getContent();
		else
		{
			stringstream content;
			Curl().readWebDocument(content, link);
			It itImported(content);
			static const ConType CON = ConType::DOCUMENT;
			static const auto& currentNamespace = Namespace::getEmptyInstance();
			if (checkEmptyContainer(itImported, CON))
				return import;

			do
			{
				switch (*itImported)
				{
				case CHAR_ABSTRACT_ENTITY:
					checkMultiContainer(++itImported, [&]() { auto ent = parseAbstractEntity(itImported, currentNamespace); body.push_back(ParamDocument(ent)); ents.addLocal(move(ent)); });
					break;
				case CHAR_CONCRETE_ENTITY:
					checkMultiContainer(++itImported, [&]() { auto ent = parseConcreteEntity(itImported, CON); body.push_back(ParamDocument(ent, true)); ents.addLocal(move(ent)); });
					break;
				case CHAR_USING_NAMESPACE:
					checkMultiContainer(++itImported, [&]() { body.push_back(parseScopedDocument(itImported)); });
					break;
				case CHAR_IMPORT:
					checkMultiContainer(++itImported, [&]() { body.push_back(parseImport(itImported, CON)); });
					break;
				default:
					throw runtime_error(ERROR_UNEXPECTED);
				}
			} while (checkNextElementContainer(itImported, CON));
		}
		return import;
	}
	catch (exception e)
	{
		throw runtime_error(string("while parsing import, ") + e.what());
	}
#endif
}