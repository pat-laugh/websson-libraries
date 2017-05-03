//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "importManager.h"
#include "paramDocumentIncluder.h"
#include "patternsContainers.h"
#include "utils/constants.h"
#include "utils/utilsWebss.h"

using namespace std;
using namespace webss;

const char ERROR_INPUT_DICTIONARY[] = "dictionary can only have key-values";
const char ERROR_INPUT_TUPLE[] = "tuple can only have concrete value-onlys or key-values";
const char ERROR_INPUT_NAMESPACE[] = "namespace can only have entity definitions";
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
		string name;
		if (nextTag == Tag::NAME_START)
			name = parseName(it);
		else if (nextTag == Tag::EXPLICIT_NAME)
			name = parseNameExplicit();
		else if (nextTag == Tag::C_STRING)
		{
			addJsonKeyvalue(dict);
			return;
		}
		else
			throw runtime_error(ERROR_INPUT_DICTIONARY);
		nextTag = getTag(it);
		dict.addSafe(move(name), parseValueOnly());
	});
}

string Parser::parseNameJson()
{
	string name;
	auto tag = getTag(++it);
	if (tag == Tag::NAME_START)
		name = parseName(it);
	else if (tag == Tag::EXPLICIT_NAME)
		name = parseNameExplicit();
	else
		throw runtime_error("expected name in supposed Json key-value");
	if (getTag(it) != Tag::C_STRING)
		throw runtime_error("expected end quote in supposed Json key-value");
	++it;
	return name;
}

void Parser::addJsonKeyvalue(Dictionary& dict)
{
	try
	{
		auto name = parseNameJson();
		skipJunkToTag(it, Tag::LINE_STRING);
		++it;
		dict.addSafe(move(name), parseValueEqual());
	}
	catch (const runtime_error&)
	{
		throw runtime_error("could not parse supposed Json key-value");
	}
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
			nspace.addSafe(parseAbstractEntity(nspace));
			break;
		case CHAR_CONCRETE_ENTITY:
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
			name = parseName(it);
		else if (nextTag == Tag::EXPLICIT_NAME)
			name = parseNameExplicit();
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
		switch (nextTag)
		{
		case Tag::ENTITY_ABSTRACT:
		{
			auto ent = parseAbstractEntity(nspace);
			docHead.push_back(ParamDocument::makeEntityAbstract(ent));
			ents.addLocalSafe(move(ent));
			break;
		}
		case Tag::ENTITY_CONCRETE:
		{
			auto ent = parseConcreteEntity();
			docHead.push_back(ParamDocument::makeEntityConcrete(ent));
			ents.addLocalSafe(move(ent));
			break;
		}
		case Tag::IMPORT:
		{
			auto import = parseImport();
			const auto& link = import.getLink();
			for (const auto& entPair : ImportManager::getInstance().importDocument(link))
				ents.addLocalSafe(entPair.second);
			docHead.push_back(move(import));
			break;
		}
		case Tag::USING_ONE:
		{
			auto param = parseUsingOne();
			const auto& ent = param.getEntity();
			ents.addLocalSafe(Entity(ent.getName(), ent.getContent()));
			docHead.push_back(move(param));
			break;
		}
		case Tag::USING_ALL:
		{
			auto param = parseUsingAll();
			const auto& nspace = param.getNamespace();

			//first check the namespace entity is accessible; if so it has to be removed since
			//it'll no longer be necessary and an entity with the same name could be inside
			if (ParamDocumentIncluder::namespacePresentScope(ents, nspace))
				ents.removeLocal(ents[nspace.getName()]);

			for (const auto& ent : nspace)
				ents.addLocalSafe(ent);

			docHead.push_back(move(param));

			break;
		}
		case Tag::SELF:
			docHead.push_back(parseScopedDocument());
			break;
		case Tag::OPTION:
			parseOption();
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

ParamDocument Parser::parseScopedDocument()
{
	skipJunkToTag(++it, Tag::START_TEMPLATE);
	auto head = parseScopedDocumentHead();
	skipJunkToTag(it, Tag::START_DICTIONARY);
	auto body = parseScopedDocumentBody(head);
	return ScopedDocument{ move(head), move(body) };
}

ParamDocument Parser::parseUsingOne()
{
	vector<string> names;
	do
		names.push_back(parseName(skipJunkToTag(++it, Tag::NAME_START)));
	while ((nextTag = getTag(it)) == Tag::SCOPE);
	if (nextTag != Tag::IMPORT)
		throw runtime_error("expected import for scoped import");
	auto import = parseImport();
	const Entity* ent = &ImportManager::getInstance().importDocument(import.getLink()).at(names[0]);
	for (decltype(names.size()) i = 1; i < names.size(); ++i)
		ent = &ent->getContent().getNamespaceSafe().at(names[i]);
	return ParamDocument::makeUsingOne(*ent, move(import));
}

ParamDocument Parser::parseUsingAll()
{
	skipJunkToTag(++it, Tag::NAME_START);
	auto nameType = parseNameType();
	if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isNamespace())
		throw runtime_error("expected namespace");
	return ParamDocument::makeUsingAll(nameType.entity);
}

ImportedDocument Parser::parseImport()
{
	nextTag = getTag(++it);
	auto importName = parseValueOnly();
	if (!importName.isString())
		throw runtime_error("import must reference a string");
	return ImportedDocument(move(importName));
}

void Parser::parseOption()
{
	skipLineJunk(++it);
	if (it != '-' || !++it)
		throw runtime_error("expected option version");
	if (*it == 'v')
		++it;
	else if (*it != '-' || !++it || !isNameStart(it) || parseName(it) != "version")
		throw runtime_error("expected option version");
	parseOptionVersion();
}

void Parser::parseOptionVersion()
{
	if (it != ':')
		throw runtime_error("expected line-string");
	++it;
	auto version = parseLineString();
	if (version != "1.0.0")
		throw runtime_error("this parser can only parse version 1.0.0");
}