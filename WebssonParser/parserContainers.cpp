//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

//#define DISABLE_IMPORT
#ifndef DISABLE_IMPORT
#include "curl.h"
#endif
#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

const char ERROR_INPUT_DICTIONARY[] = "dictionary can only have key-values";
const char ERROR_INPUT_LIST[] = "list can only have concrete value-onlys";
const char ERROR_INPUT_TUPLE[] = "tuple can only have concrete value-onlys or key-values";
const char ERROR_INPUT_NAMESPACE[] = "namespace can only have entity definitions";
const char ERROR_INPUT_ENUM[] = "enum can only have key-onlys";
const char ERROR_INPUT_DOCUMENT[] = "document can only have concrete value-onlys or key-values";

string getItPosition(SmartIterator& it)
{
	return "[ln " + to_string(it.getLine()) + ", ch " + to_string(it.getCharCount()) + "]";
}

string getItCurrentChar(SmartIterator& it)
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

Dictionary GlobalParser::Parser::parseDictionary(It& it)
{
	return parseContainer<Dictionary, ConType::DICTIONARY>(it, Dictionary(), [&](Dictionary& dict, ConType con)
	{
		if (*it == CHAR_CSTRING)
			addJsonKeyvalue(++it, dict);
		else
			parseOtherValue(it, con,
				CaseKeyValue{ dict.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_DICTIONARY),
				ErrorValueOnly(ERROR_INPUT_DICTIONARY),
				ErrorAbstractEntity(ERROR_INPUT_DICTIONARY));
	});
}

List GlobalParser::Parser::parseList(It& it)
{
	return parseContainer<List, ConType::LIST>(it, List(), [&](List& list, ConType con)
	{
		list.add(parseValueOnly(it, con));
	});
}

Tuple GlobalParser::Parser::parseTuple(It& it)
{
	return parseContainer<Tuple, ConType::TUPLE>(it, Tuple(), [&](Tuple& tuple, ConType con)
	{
		parseOtherValue(it, con,
			CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
			ErrorKeyOnly(ERROR_INPUT_TUPLE),
			CaseValueOnly{ tuple.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_TUPLE));
	});
}

List GlobalParser::Parser::parseListText(It& it)
{
	return parseContainer<List, ConType::LIST>(it, List(), [&](List& list, ConType con)
	{
		list.add(parseLineString(it, con));
	});
}
Tuple GlobalParser::Parser::parseTupleText(It& it)
{
	return parseContainer<Tuple, ConType::TUPLE>(it, Tuple(), [&](Tuple& tuple, ConType con)
	{
		tuple.add(parseLineString(it, con));
	});
}

Namespace GlobalParser::Parser::parseNamespace(It& it, const string& name, const Namespace& previousNamespace)
{
	return parseContainer<Namespace, ConType::DICTIONARY>(it, Namespace(name, previousNamespace), [&](Namespace& nspace, ConType con)
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			nspace.addSafe(parseAbstractEntity(++it, nspace));
			break;
		case CHAR_CONCRETE_ENTITY:
			nspace.addSafe(parseConcreteEntity(++it, con));
			break;
		case CHAR_SELF:
			skipJunkToTag(++it, Tag::START_TEMPLATE);
			nspace.addSafe(Entity(string(name), parseTemplateHead(++it)));
			break;
		default:
			throw runtime_error(ERROR_INPUT_NAMESPACE);
		}
	});
}
Enum GlobalParser::Parser::parseEnum(It& it, const string& name)
{
	return parseContainer<Enum, ConType::LIST>(it, Enum(name), [&](Enum& tEnum, ConType con)
	{
		parseOtherValue(it, con,
			ErrorKeyValue(ERROR_INPUT_ENUM),
			CaseKeyOnly{ tEnum.add(move(key)); },
			ErrorValueOnly(ERROR_INPUT_ENUM),
			ErrorAbstractEntity(ERROR_INPUT_ENUM));
	});
}

Document GlobalParser::Parser::parseDocument(It& it)
{
	static const ConType CON = ConType::DOCUMENT;
	try
	{
		Document doc;
		if (!checkEmptyContainer(it, CON) && !parseDocumentHead(it, doc.getHead(), CON, Namespace::getEmptyInstance()))
		{
			do
				parseOtherValue(it, CON,
					CaseKeyValue{ doc.addSafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
					CaseValueOnly{ doc.add(move(value)); },
					ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
			while (checkNextElementContainer(it, CON));
		}
		return doc;
	}
	catch (const exception& e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
}

bool GlobalParser::Parser::parseDocumentHead(It& it, vector<ParamDocument>& docHead, ConType con, const Namespace& nspace)
{
	assert(it);
	do
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
		{
			auto ent = parseAbstractEntity(++it, nspace);
			docHead.push_back(ParamDocument::makeEntityAbstract(ent)); ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_CONCRETE_ENTITY:
		{
			auto ent = parseConcreteEntity(++it, con);
			docHead.push_back(ParamDocument::makeEntityConcrete(ent)); ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_IMPORT:
		{
			auto import = parseImport(++it, con);
			docHead.push_back(move(import));
			break;
		}
		case CHAR_SCOPED_DOCUMENT:
			parseScopedDocument(++it, docHead);
			break;
		default:
			return false;
		}
	} while (checkNextElementContainer(it, con));
	return true;
}

void GlobalParser::Parser::parseScopedDocument(It& it, vector<ParamDocument>& docHead)
{
	static const ConType CON = ConType::DICTIONARY;
	if (*skipJunkToValid(it) == OPEN_TEMPLATE)
	{
		auto head = parseTemplateHeadScoped(++it);

		skipJunkToTag(it, Tag::START_DICTIONARY);
		DocumentHead body;
		if (!checkEmptyContainer(++it, CON))
		{
			ParamDocumentIncluder includer(ents, head.getParameters());
			if (!parseDocumentHead(it, body, CON, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
		}
		docHead.push_back(ScopedDocument{ move(head), move(body) });
	}
	else
	{
		const auto& nspace = parseUsingNamespaceStatic(it);

		//first check the namespace entity is accessible; if so it has to be removed since
		//it'll no longer be necessary and an entity with the same name could be inside
		if (ParamDocumentIncluder::namespacePresentScope(ents, nspace))
			ents.removeLocal(ents[nspace.getName()]);

		for (const auto& ent : nspace)
			ents.addLocalSafe(ent);
	}
}

ImportedDocument GlobalParser::Parser::parseImport(It& it, ConType con)
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	auto importName = parseValueOnly(it, con);
	if (!importName.isString())
		throw runtime_error("import must reference a string");
	ImportedDocument import(move(importName));
	const auto& link = import.getLink();
	if (!importedDocuments.hasEntity(link))
	{
		try
		{
			importedDocuments.addLocalSafe(link, 0);
			static const ConType CON = ConType::DOCUMENT;
			It itImported(Curl().readWebDocument(link));
			DocumentHead docHead;
			if (!checkEmptyContainer(itImported, CON) && !parseDocumentHead(itImported, docHead, CON, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
		}
		catch (const exception& e)
			{ throw runtime_error(string("while parsing import, ") + e.what()); }
	}
	return import;
#endif
}

const Namespace& GlobalParser::Parser::parseUsingNamespaceStatic(It& it)
{
	auto nameType = parseNameType(skipJunkToTag(it, Tag::NAME_START));
	if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isNamespace())
		throw runtime_error("expected namespace");
	return nameType.entity.getContent().getNamespaceSafe();
}