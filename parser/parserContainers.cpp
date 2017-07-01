//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "importManager.hpp"
#include "iteratorSwitcher.hpp"
#include "nameType.hpp"
#include "parserStrings.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utilsOptions.hpp"
#include "utilsTemplateDefaultValues.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace various;
using namespace webss;

const char ERROR_INPUT_DICTIONARY[] = "dictionary can only have key-values";
const char ERROR_INPUT_TUPLE[] = "tuple can only have concrete value-onlys or key-values";
const char ERROR_INPUT_NAMESPACE[] = "namespace can only have entity definitions";
const char ERROR_INPUT_DOCUMENT[] = "document can only have concrete value-onlys or key-values";
const char ERROR_OPTION[] = "expected option";

void useNamespace(EntityManager& ents, const Namespace& nspace)
{
	for (auto ent : nspace)
	{
		ent.removeNamespace();
		ents.addPublicSafe(ent);
	}
}

string getItPosition(const SmartIterator& it)
{
	return "[ln " + to_string(it.getLine()) + ", ch " + to_string(it.getCharCount()) + "]";
}

string getItCurrentChar(const SmartIterator& it)
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
		if (*tagit == Tag::EXPAND)
			expandDictionary(dict, tagit, ents);
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
		switch (*tagit)
		{
		case Tag::EXPAND:
			expandList(list, tagit, ents);
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
		switch (*tagit)
		{
		case Tag::EXPAND:
			expandTuple(tuple, tagit, ents);
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
		switch (*tagit)
		{
		case Tag::EXPAND:
			expandNamespace(nspace, tagit, ents);
			break;
		case Tag::ENTITY_ABSTRACT:
			nspace.addSafe(parseAbstractEntity(nspace));
			break;
		case Tag::ENTITY_CONCRETE:
			nspace.addSafe(parseConcreteEntity());
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
		switch (*tagit)
		{
		case Tag::EXPAND:
			expandEnum(tEnum, tagit, ents);
			break;
		case Tag::NAME_START:
			tEnum.addSafe(parseName(getItSafe()));
			break;
		case Tag::EXPLICIT_NAME:
			tEnum.addSafe(parseNameExplicit(tagit));
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
				switch (*tagit)
				{
				case Tag::EXPAND:
					expandDocumentBody(doc, tagit, ents);
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
		throw runtime_error(string(getItPosition(getIt()) + ' ' + e.what() + getItCurrentChar(getIt())).c_str());
	}
}

bool Parser::parseDocumentHead(Document& doc, const Namespace& nspace)
{
	auto& docHead = doc.getHead();
	do
	{
		switch (*tagit)
		{
		case Tag::ENTITY_ABSTRACT:
		{
			auto ent = parseAbstractEntity(nspace);
			docHead.push_back(ParamDocument::makeEntityAbstract(ent));
			ents.addPublicSafe(move(ent));
			break;
		}
		case Tag::ENTITY_CONCRETE:
		{
			auto ent = parseConcreteEntity();
			docHead.push_back(ParamDocument::makeEntityConcrete(ent));
			ents.addPublicSafe(move(ent));
			break;
		}
		case Tag::IMPORT:
		{
			auto import = parseImport();
			const auto& link = import.getLink();
			for (const auto& entPair : ImportManager::getInstance().importDocument(link))
				ents.addPublicSafe(entPair.second);
			docHead.push_back(move(import));
			break;
		}
		case Tag::SCOPED_IMPORT:
			docHead.push_back(parseScopedImport());
			break;
		case Tag::EXPAND:
		{
			auto ent = parseExpandEntity(tagit, ents);
			if (ent.getContent().getType() != WebssType::NAMESPACE)
			{
				if (ent.getContent().getType() != WebssType::TUPLE && ent.getContent().getType() != WebssType::TUPLE_TEXT)
					throw runtime_error("expand entity in document body must be a tuple");
				for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
					item.first == nullptr ? doc.add(*item.second) : doc.addSafe(*item.first, *item.second);
				tagit.getSafe();
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

vector<string> parseScopedImportNames(TagIterator& tagit)
{
	assert(*tagit == Tag::NAME_START);
	vector<string> names;
	names.push_back(parseName(tagit.getIt()));
	while (tagit.update() == Tag::SCOPE)
	{
		(++tagit).sofertTag(Tag::NAME_START);
		names.push_back(parseName(tagit.getIt()));
	}
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
	if (++tagit == Tag::NAME_START)
	{
		auto names = parseScopedImportNames(tagit);

		tagit.sofertTag(Tag::IMPORT);
		auto import = parseImport();
		const auto& importedDoc = ImportManager::getInstance().importDocument(import.getLink());

		auto ent = getScopedImportEntity(importedDoc, names);
		ents.addPublicSafe(ent);

		return ParamDocument::makeScopedImport(move(ent), move(import));
	}
	else if (*tagit == Tag::START_LIST)
	{
		auto namesList = parseContainer<vector<vector<string>>, ConType::LIST>(vector<vector<string>>(), false, [&](vector<vector<string>>& namesList)
		{
			tagit.sofertTag(Tag::NAME_START);
			namesList.push_back(parseScopedImportNames(tagit));
		});

		tagit.sofertTag(Tag::IMPORT);
		auto import = parseImport();
		const auto& importedDoc = ImportManager::getInstance().importDocument(import.getLink());

		vector<Entity> entList;
		for (const auto& names : namesList)
		{
			auto ent = getScopedImportEntity(importedDoc, names);
			ents.addPublicSafe(ent);
			entList.push_back(move(ent));
		}

		return ParamDocument::makeScopedImport(move(entList), move(import));
	}
	else
		throw runtime_error(ERROR_UNEXPECTED);
}

static Thead makeTheadImport()
{
	TheadStd thead;
	thead.attachEmpty("name");
	thead.attach("location", ParamStd("Std"));
	thead.attach("version", ParamStd("1"));
	return Thead(move(thead));
}

ImportedDocument Parser::parseImport()
{
	static const auto thead = makeTheadImport();
	const auto& params = thead.getTheadStd().getParams();
	switch (++tagit)
	{
//	case Tag::START_TUPLE:
//		return ImportedDocument(parseTemplateTupleStd(thead));
//	case Tag::TEXT_TUPLE:
//		return ImportedDocument(Webss(parseTemplateTupleText(thead), WebssType::TUPLE_TEXT));
	case Tag::NAME_START: case Tag::SCOPE: case Tag::SLASH:
	{
		Tuple tuple(params.getSharedKeys());
		tuple[0] = parseStickyLineString(*this);
		checkDefaultValues(tuple, params);
		return ImportedDocument(move(tuple));
	}
/*	case Tag::EXPAND:
	{
		auto content = parseExpandEntity(tagit, ents).getContent();
		if (content.isString())
		{
			Tuple tuple(params.getSharedKeys());
			tuple[0] = content.getString();
			checkDefaultValues(tuple, params);
			return ImportedDocument(move(tuple));
		}
		else if (content.isTuple())
		{
			Tuple tuple;
			fillTemplateBodyTuple(params, content.getTuple(), tuple);
			checkDefaultValues(tuple, params);
			for (const auto& item : tuple)
				if (!item.isString())
					throw runtime_error(ERROR_UNEXPECTED);
			return ImportedDocument(move(tuple));
		}
		else
			throw runtime_error(ERROR_UNEXPECTED);
	} */
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

void Parser::parseOption()
{
	++getItSafe();
	auto items = parseOptionLine(*this, [](char c) { return c == '\n'; });

	for (decltype(items.size()) i = 0; i < items.size(); ++i)
	{
		if (items[i] != OPTION_NAME)
			throw runtime_error(ERROR_OPTION);
		const auto& op = items[++i];
		if (op == "a" || op == "alias")
			parseOptionAlias(items, i);
		else if (op == "v" || op == "version")
			parseOptionVersion(items, i);
		else
			throw runtime_error(ERROR_OPTION);
	}
}

void Parser::parseOptionVersion(const std::vector<std::string>& items, std::vector<std::string>::size_type& index)
{
	if (++index >= items.size() || items[index] != OPTION_VALUE)
		throw runtime_error("expected version");
	if (items[++index] != "1.0.0")
		throw runtime_error("this parser can only parse version 1.0.0");
}

void Parser::parseOptionAlias(const std::vector<std::string>& items, std::vector<std::string>::size_type& index)
{
	if (++index >= items.size() || items[index] != OPTION_SCOPE)
		throw runtime_error("expected alias");
	auto name = items[++index];
	if (hasAlias(name))
		throw runtime_error("alias already exists");
	if (++index >= items.size() || items[index] != OPTION_VALUE)
		throw runtime_error("expected alias value");
	aliases.insert({ move(name), expandOptionString(*this, items[++index]) });
}