//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "importManager.hpp"
#include "nameType.hpp"
#include "paramDocumentIncluder.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

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

string Parser::parseNameDictionary()
{
	if (nextTag == Tag::NAME_START)
		return parseName(it);
	else if (nextTag == Tag::EXPLICIT_NAME)
		return parseNameExplicit();
	throw runtime_error(ERROR_INPUT_DICTIONARY);
}

Dictionary Parser::parseDictionary()
{
	return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), false, [&](Dictionary& dict)
	{
		if (nextTag == Tag::EXPAND)
			expandDictionary(dict, it, ents);
		else
		{
			string name = parseNameDictionary();
			nextTag = getTag(it);
			dict.addSafe(move(name), parseValueOnly());
		}
	});
}

List Parser::parseList()
{
	return parseContainer<List, ConType::LIST>(List(), false, [&](List& list)
	{
		if (nextTag == Tag::EXPAND)
			expandList(list, it, ents);
		else
			list.add(parseValueOnly());
	});
}

List Parser::parseListText()
{
	return parseContainer<List, ConType::LIST>(List(), false, [&](List& list)
	{
		if (nextTag == Tag::EXPAND)
			expandList(list, it, ents);
		else
			list.add(parseLineString());
	});
}

Tuple Parser::parseTuple()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), false, [&](Tuple& tuple)
	{
		if (nextTag == Tag::EXPAND)
			expandTuple(tuple, it, ents);
		else if (nextTag == Tag::EXPLICIT_NAME)
		{
			parseOtherValue(
				CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_TUPLE),
				ErrorValueOnly(ERROR_INPUT_TUPLE),
				ErrorAbstractEntity(ERROR_INPUT_TUPLE));
		}
		else
		{
			parseOtherValue(
				CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_TUPLE),
				CaseValueOnly{ tuple.add(move(value)); },
				ErrorAbstractEntity(ERROR_INPUT_TUPLE));
		}
	});
}

Tuple Parser::parseTupleText()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), false, [&](Tuple& tuple)
	{
		if (nextTag == Tag::EXPAND)
			expandTuple(tuple, it, ents);
		else if (nextTag == Tag::EXPLICIT_NAME)
		{
			parseOtherValue(
				CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_TUPLE),
				ErrorValueOnly(ERROR_INPUT_TUPLE),
				ErrorAbstractEntity(ERROR_INPUT_TUPLE));
		}
		else
			tuple.add(parseLineString());
	});
}

Tuple Parser::parseTupleAbstract()
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), true, [&](Tuple& tuple)
	{
		switch (nextTag)
		{
		case Tag::SEPARATOR: //void
			tuple.add(Webss());
			break;
		case Tag::EXPAND:
			expandTuple(tuple, it, ents, true);
			break;
		default:
			parseOtherValue(
				CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_TUPLE),
				CaseValueOnly{ tuple.add(move(value)); },
				ErrorAbstractEntity(ERROR_INPUT_TUPLE));
			break;
		}
	});
}

Namespace Parser::parseNamespace(const string& name, const Namespace& previousNamespace)
{
	return parseContainer<Namespace, ConType::DICTIONARY>(Namespace(name, previousNamespace), false, [&](Namespace& nspace)
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
	return parseContainer<Enum, ConType::LIST>(Enum(name), false, [&](Enum& tEnum)
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
		ent = &ent->getContent().getNamespace().at(names[i]);
	return ParamDocument::makeUsingOne(*ent, move(import));
}

ParamDocument Parser::parseUsingAll()
{
	skipJunkToTag(++it, Tag::NAME_START);
	auto nameType = parseNameType(it, ents);
	if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isNamespace())
		throw runtime_error("expected namespace");
	return ParamDocument::makeUsingAll(nameType.entity);
}

TemplateHeadStandard makeTheadImport()
{
	TemplateHeadStandard thead;
	thead.attachEmpty("name");
	thead.attach("location", ParamStandard("Standard"));
	thead.attach("version", ParamStandard("1"));
	return thead;
}

ImportedDocument Parser::parseImport()
{
	static const auto thead = makeTheadImport();
	nextTag = getTag(++it);
	auto importTuple = parseTemplateTupleText(thead.getParameters());
	auto importName = importTuple["name"];
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