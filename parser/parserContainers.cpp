//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "importManager.hpp"
#include "nameType.hpp"
#include "parserStrings.hpp"
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

bool namespaceNameInCurrentScope(const EntityManager& ents, const Namespace& nspace);
void useNamespace(EntityManager& ents, const Namespace& nspace);

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
	return parseContainer<Dictionary, ConType::DICTIONARY>(Dictionary(), false, [&](Dictionary& dict)
	{
		if (nextTag == Tag::EXPAND)
			expandDictionary(dict, it, ents);
		else
			parseExplicitKeyValue(
				CaseKeyValue{ dict.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_DICTIONARY));
	});
}

List Parser::parseListCommon(function<void(List&)> defaultFunc)
{
	return parseContainer<List, ConType::LIST>(List(), false, [&](List& list)
	{
		switch (nextTag)
		{
		case Tag::EXPAND:
			expandList(list, it, ents);
			break;
		case Tag::EXPLICIT_NAME:
			throw runtime_error("list can only contain values");
		default:
			defaultFunc(list);
			break;
		}
	});
}

List Parser::parseList()
{
	return parseListCommon([&](List& list)
	{
		list.add(parseValueOnly());
	});
}

List Parser::parseListText()
{
	return parseListCommon([&](List& list)
	{
		list.add(parseLineString(*this));
	});
}

Tuple Parser::parseTupleCommon(function<void(Tuple&)> defaultFunc)
{
	return parseContainer<Tuple, ConType::TUPLE>(Tuple(), false, [&](Tuple& tuple)
	{
		switch (nextTag)
		{
		case Tag::EXPAND:
			expandTuple(tuple, it, ents);
			break;
		case Tag::EXPLICIT_NAME:
			parseExplicitKeyValue(
				CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
				ErrorKeyOnly(ERROR_INPUT_TUPLE));
			break;
		default:
			defaultFunc(tuple);
			break;
		}
	});
}

Tuple Parser::parseTuple()
{
	return parseTupleCommon([&](Tuple& tuple)
	{
		parseOtherValue(
			CaseKeyValue{ tuple.addSafe(move(key), move(value)); },
			ErrorKeyOnly(ERROR_INPUT_TUPLE),
			CaseValueOnly{ tuple.add(move(value)); },
			ErrorAbstractEntity(ERROR_INPUT_TUPLE));
	});
}

Tuple Parser::parseTupleText()
{
	return parseTupleCommon([&](Tuple& tuple)
	{
		tuple.add(parseLineString(*this));
	});
}

Namespace Parser::parseNamespace(const string& name, const Namespace& previousNamespace)
{
	return parseContainer<Namespace, ConType::DICTIONARY>(Namespace(name, previousNamespace), false, [&](Namespace& nspace)
	{
		switch (nextTag)
		{
		case Tag::EXPAND:
			expandNamespace(nspace, it, ents);
			break;
		case Tag::ENTITY_ABSTRACT:
			nspace.addSafe(parseAbstractEntity(nspace));
			break;
		case Tag::ENTITY_CONCRETE:
			nspace.addSafe(parseConcreteEntity());
			break;
		case Tag::SELF:
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
		switch (nextTag)
		{
		case Tag::EXPAND:
			expandEnum(tEnum, it, ents);
			break;
		case Tag::NAME_START:
			tEnum.addSafe(parseName(it));
			break;
		case Tag::EXPLICIT_NAME:
			tEnum.addSafe(parseNameExplicit(it));
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	});
}

Document Parser::parseDocument()
{
	try
	{
		Document doc;
		if (!containerEmpty() && !parseDocumentHead(doc, Namespace::getEmptyInstance()))
		{
			do
			{
				switch (nextTag)
				{
				case Tag::EXPAND:
					expandDocumentBody(doc, it, ents);
					break;
				case Tag::EXPLICIT_NAME:
					parseExplicitKeyValue(
						CaseKeyValue{ doc.addSafe(move(key), move(value)); },
						ErrorKeyOnly(ERROR_INPUT_DOCUMENT));
					break;
				default:
					parseOtherValue(
						CaseKeyValue{ doc.addSafe(move(key), move(value)); },
						ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
						CaseValueOnly{ doc.add(move(value)); },
						ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
					break;
				}
			} while (checkNextElement());
		}
		return doc;
	}
	catch (const exception& e)
	{
		throw runtime_error(string(getItPosition(it) + ' ' + e.what() + getItCurrentChar(it)).c_str());
	}
}

bool Parser::parseDocumentHead(Document& doc, const Namespace& nspace)
{
	auto& docHead = doc.getHead();
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
		case Tag::SCOPED_IMPORT:
			docHead.push_back(parseScopedImport());
			break;
		case Tag::EXPAND:
		{
			auto ent = parseExpandEntity(it, ents);
			if (ent.getContent().getType() != WebssType::NAMESPACE)
			{
				if (ent.getContent().getType() != WebssType::TUPLE && ent.getContent().getType() != WebssType::TUPLE_TEXT)
					throw runtime_error("expand entity in document body must be a tuple");
				for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
					item.first == nullptr ? doc.add(*item.second) : doc.addSafe(*item.first, *item.second);
				return false;
			}
			auto param = ParamDocument::makeExpand(ent);
			const auto& nspace = param.getNamespace();
			useNamespace(ents, nspace);
			docHead.push_back(move(param));
			break;
		}
		case Tag::OPTION:
			parseOption();
			break;
		default:
			return false;
		}
	} while (checkNextElement());
	return true;
}

vector<string> parseScopedImportNames(SmartIterator& it)
{
	vector<string> names;
	names.push_back(parseName(it));
	while (getTag(it) == Tag::SCOPE)
		names.push_back(parseName(skipJunkToTag(++it, Tag::NAME_START)));
	return names;
}

Entity getScopedImportEntity(const unordered_map<string, Entity>& importedDoc, vector<string> names)
{
	const Entity* ent = &importedDoc.at(names[0]);
	for (decltype(names.size()) i = 1; i < names.size(); ++i)
		ent = &ent->getContent().getNamespace().at(names[i]);
	return *ent;
}

ParamDocument Parser::parseScopedImport()
{
	nextTag = getTag(++it);
	if (nextTag == Tag::NAME_START)
	{
		auto names = parseScopedImportNames(it);

		if (getTag(it) != Tag::IMPORT)
			throw runtime_error("expected import for scoped import");
		auto import = parseImport();
		const auto& importedDoc = ImportManager::getInstance().importDocument(import.getLink());

		auto ent = getScopedImportEntity(importedDoc, names);
		ents.addLocalSafe(ent);

		return ParamDocument::makeScopedImport(move(ent), move(import));
	}
	else if (nextTag == Tag::START_LIST)
	{
		auto namesList = parseContainer<vector<vector<string>>, ConType::LIST>(vector<vector<string>>(), false, [&](vector<vector<string>>& namesList)
		{
			if (nextTag != Tag::NAME_START)
				throw runtime_error(ERROR_UNEXPECTED);
			namesList.push_back(parseScopedImportNames(it));
		});

		if (getTag(it) != Tag::IMPORT)
			throw runtime_error("expected import for scoped import");
		auto import = parseImport();
		const auto& importedDoc = ImportManager::getInstance().importDocument(import.getLink());

		vector<Entity> entList;
		for (const auto& names : namesList)
		{
			auto ent = getScopedImportEntity(importedDoc, names);
			ents.addLocalSafe(ent);
			entList.push_back(move(ent));
		}

		return ParamDocument::makeScopedImport(move(entList), move(import));
	}
	else
		throw runtime_error(ERROR_UNEXPECTED);
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
	auto version = parseLineString(*this);
	if (version != "1.0.0")
		throw runtime_error("this parser can only parse version 1.0.0");
}

bool namespaceNameInCurrentScope(const EntityManager& ents, const Namespace& nspace)
{
	const auto& name = nspace.getName();
	if (!ents.hasEntity(name))
		return false;

	//make sure they're the exact same entity, not just two different entities with the same name
	const auto& content = ents[name].getContent();
	return content.isNamespace() && content.getNamespace() == nspace;
}

void useNamespace(EntityManager& ents, const Namespace& nspace)
{
	//if namespace entity is accessible, it has to be removed since
	//it'll no longer be necessary and an entity with the same name could be inside
	if (namespaceNameInCurrentScope(ents, nspace))
		ents.removeLocal(ents[nspace.getName()]);

	for (const auto& ent : nspace)
		ents.addLocalSafe(ent);
}