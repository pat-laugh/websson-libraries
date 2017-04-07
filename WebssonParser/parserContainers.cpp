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

Dictionary Parser::parseDictionary()
{
	return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), [&](Dictionary& dict)
	{
		if (nextTag == Tag::C_STRING)
			addJsonKeyvalue(dict);
		else
			parseOtherValue(
				CaseKeyValue{ dict.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_DICTIONARY),
				ErrorValueOnly(ERROR_INPUT_DICTIONARY),
				ErrorAbstractEntity(ERROR_INPUT_DICTIONARY));
	});
}

List Parser::parseList()
{
	return parseContainer<List, ConType::LIST>(List(), [&](List& list)
	{
		list.add(parseValueOnly());
	});
}

Tuple Parser::parseTuple()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), [&](Tuple& tuple)
	{
		parseOtherValue(
			CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
			ErrorKeyOnly(ERROR_INPUT_TUPLE),
			CaseValueOnly{ tuple.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_TUPLE));
	});
}

List Parser::parseListText()
{
	return parseContainer<List, ConType::LIST>(List(), [&](List& list)
	{
		list.add(parseLineString());
	});
}
Tuple Parser::parseTupleText()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), [&](Tuple& tuple)
	{
		tuple.add(parseLineString());
	});
}

Namespace Parser::parseNamespace(const string& name, const Namespace& previousNamespace)
{
	return parseContainer<Namespace, ConType::DICTIONARY>(Namespace(name, previousNamespace), [&](Namespace& nspace)
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			++it;
			nspace.addSafe(parseAbstractEntity(nspace));
			break;
		case CHAR_CONCRETE_ENTITY:
			++it;
			nspace.addSafe(parseConcreteEntity());
			break;
		case CHAR_SELF:
			skipJunkToTag(++it, Tag::START_TEMPLATE);
			nspace.addSafe(Entity(string(name), parseTemplateHead()));
			break;
		default:
			throw runtime_error(ERROR_INPUT_NAMESPACE);
		}
	});
}

Enum Parser::parseEnum(const string& name)
{
	return parseContainer<Enum, ConType::LIST>(Enum(name), [&](Enum& tEnum)
	{
		string name;
		if (nextTag == Tag::NAME_START)
			name = parseNameNotKeyword();
		else if (nextTag == Tag::EXPLICIT_NAME)
			name = parseExplicitName();
		else
			throw runtime_error(ERROR_UNEXPECTED);
		tEnum.addSafe(move(name));
	});
}

Document Parser::parseDocument()
{
	try
	{
		Document doc;
		if (!containerEmpty() && !parseDocumentHead(doc.getHead(), Namespace::getEmptyInstance()))
		{
			do
				parseOtherValue(
					CaseKeyValue{ doc.addSafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
					CaseValueOnly{ doc.add(move(value)); },
					ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
			while (checkNextElement());
		}
		return doc;
	}
	catch (const exception& e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
}

bool Parser::parseDocumentHead(vector<ParamDocument>& docHead, const Namespace& nspace)
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
			docHead.push_back(ParamDocument::makeEntityAbstract(ent));
			ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_CONCRETE_ENTITY:
		{
			++it;
			auto ent = parseConcreteEntity();
			docHead.push_back(ParamDocument::makeEntityConcrete(ent));
			ents.addLocalSafe(move(ent));
			break;
		}
		case CHAR_IMPORT:
		{
			++it;
			auto import = parseImport();
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
	} while (checkNextElement());
	return true;
}

TemplateHeadScoped Parser::parseScopedDocumentHead()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		throw runtime_error("can't have empty scoped document head");
	return parseTemplateHeadScoped();
}

DocumentHead Parser::parseScopedDocumentBody(const TemplateHeadScoped& head)
{
	DocumentHead body;
	ContainerSwitcher switcher(*this, ConType::DICTIONARY, false);
	if (!containerEmpty())
	{
		ParamDocumentIncluder includer(ents, head.getParameters());
		if (!parseDocumentHead(body, Namespace::getEmptyInstance()))
			throw runtime_error(ERROR_UNEXPECTED);
	}
	return body;
}

void Parser::parseScopedDocument(vector<ParamDocument>& docHead)
{
	if (*skipJunkToValid(it) == OPEN_TEMPLATE)
	{
		auto head = parseScopedDocumentHead();
		skipJunkToTag(it, Tag::START_DICTIONARY);
		auto body = parseScopedDocumentBody(head);
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

ImportedDocument Parser::parseImport()
{
#ifdef DISABLE_IMPORT
	throw runtime_error("this parser cannot import documents");
#else
	nextTag = getTag(it);
	auto importName = parseValueOnly();
	if (!importName.isString())
		throw runtime_error("import must reference a string");
	ImportedDocument import(move(importName));
	const auto& link = import.getLink();
	if (!importedDocuments.hasEntity(link))
	{
		try
		{
			importedDocuments.addLocalSafe(link, 0);
			ImportSwitcher switcher(*this, SmartIterator(Curl().readWebDocument(link)));
			DocumentHead docHead;
			if (!containerEmpty() && !parseDocumentHead(docHead, Namespace::getEmptyInstance()))
				throw runtime_error(ERROR_UNEXPECTED);
		}
		catch (const exception& e)
			{ throw runtime_error(string("while parsing import, ") + e.what()); }
	}
	return import;
#endif
}

const Namespace& Parser::parseUsingNamespaceStatic()
{
	skipJunkToTag(it, Tag::NAME_START);
	auto nameType = parseNameType();
	if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isNamespace())
		throw runtime_error("expected namespace");
	return nameType.entity.getContent().getNamespaceSafe();
}