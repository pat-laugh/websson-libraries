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

Dictionary GlobalParser::Parser::parseDictionary()
{
	return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), [&](Dictionary& dict, Parser& parser)
	{
		if (*it == CHAR_CSTRING)
		{
			++it;
			parser.addJsonKeyvalue(dict);
		}
		else
			parser.parseOtherValue(parser.con,
				CaseKeyValue{ dict.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_DICTIONARY),
				ErrorValueOnly(ERROR_INPUT_DICTIONARY),
				ErrorAbstractEntity(ERROR_INPUT_DICTIONARY));
	});
}

List GlobalParser::Parser::parseList()
{
	Parser parser(*this, ConType::LIST, false);
	List cont;
	if (!parser.parserContainerEmpty())
		do
			cont.add(parseValueOnly(parser.con));
		while (parser.parserCheckNextElement());
	return cont;
}

Tuple GlobalParser::Parser::parseTuple()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), [&](Tuple& tuple, Parser& parser)
	{
		parser.parseOtherValue(parser.con,
			CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
			ErrorKeyOnly(ERROR_INPUT_TUPLE),
			CaseValueOnly{ tuple.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_TUPLE));
	});
}

List GlobalParser::Parser::parseListText()
{
	return parseContainer<List, ConType::LIST>(List(), [&](List& list, Parser& parser)
	{
		list.add(parser.parseLineString(parser.con));
	});
}
Tuple GlobalParser::Parser::parseTupleText()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), [&](Tuple& tuple, Parser& parser)
	{
		tuple.add(parser.parseLineString(parser.con));
	});
}

Namespace GlobalParser::Parser::parseNamespace(const string& name, const Namespace& previousNamespace)
{
	return parseContainer<Namespace, ConType::DICTIONARY>(Namespace(name, previousNamespace), [&](Namespace& nspace, Parser& parser)
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			++it;
			nspace.addSafe(parser.parseAbstractEntity(nspace));
			break;
		case CHAR_CONCRETE_ENTITY:
			++it;
			nspace.addSafe(parser.parseConcreteEntity(parser.con));
			break;
		case CHAR_SELF:
			skipJunkToTag(++it, Tag::START_TEMPLATE);
			++it;
			nspace.addSafe(Entity(string(name), parser.parseTemplateHead()));
			break;
		default:
			throw runtime_error(ERROR_INPUT_NAMESPACE);
		}
	});
}
Enum GlobalParser::Parser::parseEnum(const string& name)
{
	return parseContainer<Enum, ConType::LIST>(Enum(name), [&](Enum& tEnum, Parser& parser)
	{
		parser.parseOtherValue(parser.con,
			ErrorKeyValue(ERROR_INPUT_ENUM),
			CaseKeyOnly{ tEnum.add(move(key)); },
			ErrorValueOnly(ERROR_INPUT_ENUM),
			ErrorAbstractEntity(ERROR_INPUT_ENUM));
	});
}

Document GlobalParser::Parser::parseDocument()
{
	static const ConType CON = ConType::DOCUMENT;
	try
	{
		Document doc;
		if (!checkEmptyContainer(CON) && !parseDocumentHead(doc.getHead(), CON, Namespace::getEmptyInstance()))
		{
			do
				parseOtherValue(CON,
					CaseKeyValue{ doc.addSafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
					CaseValueOnly{ doc.add(move(value)); },
					ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
			while (checkNextElementContainer(CON));
		}
		return doc;
	}
	catch (const exception& e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
}

bool GlobalParser::Parser::parseDocumentHead(vector<ParamDocument>& docHead, ConType con, const Namespace& nspace)
{
	assert(it);
	do
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
		{
			++it;
			auto ent = parseAbstractEntity(nspace);
			docHead.push_back(ParamDocument::makeEntityAbstract(ent)); ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_CONCRETE_ENTITY:
		{
			++it;
			auto ent = parseConcreteEntity(con);
			docHead.push_back(ParamDocument::makeEntityConcrete(ent)); ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_IMPORT:
		{
			++it;
			auto import = parseImport(con);
			docHead.push_back(move(import));
			break;
		}
		case CHAR_SCOPED_DOCUMENT:
			++it;
			parseScopedDocument(docHead);
			break;
		default:
			return false;
		}
	} while (checkNextElementContainer(con));
	return true;
}

void GlobalParser::Parser::parseScopedDocument(vector<ParamDocument>& docHead)
{
	static const ConType CON = ConType::DICTIONARY;
	if (*skipJunkToValid(it) == OPEN_TEMPLATE)
	{
		++it;
		auto head = parseTemplateHeadScoped();

		skipJunkToTag(it, Tag::START_DICTIONARY);
		DocumentHead body;
		++it;
		if (!checkEmptyContainer(CON))
		{
			ParamDocumentIncluder includer(ents, head.getParameters());
			if (!parseDocumentHead(body, CON, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
		}
		docHead.push_back(ScopedDocument{ move(head), move(body) });
	}
	else
	{
		const auto& nspace = parseUsingNamespaceStatic();

		//first check the namespace entity is accessible; if so it has to be removed since
		//it'll no longer be necessary and an entity with the same name could be inside
		if (ParamDocumentIncluder::namespacePresentScope(ents, nspace))
			ents.removeLocal(ents[nspace.getName()]);

		for (const auto& ent : nspace)
			ents.addLocalSafe(ent);
	}
}

ImportedDocument GlobalParser::Parser::parseImport(ConType con)
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	auto importName = parseValueOnly(con);
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
			SmartIterator itImported(Curl().readWebDocument(link));
			auto parserImported = makeImportParser(itImported);
			DocumentHead docHead;
			if (!parserImported.checkEmptyContainer(CON) && !parserImported.parseDocumentHead(docHead, CON, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
		}
		catch (const exception& e)
			{ throw runtime_error(string("while parsing import, ") + e.what()); }
	}
	return import;
#endif
}

const Namespace& GlobalParser::Parser::parseUsingNamespaceStatic()
{
	skipJunkToTag(it, Tag::NAME_START);
	auto nameType = parseNameType();
	if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isNamespace())
		throw runtime_error("expected namespace");
	return nameType.entity.getContent().getNamespaceSafe();
}