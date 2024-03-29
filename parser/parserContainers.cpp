//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parserStrings.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utilsTemplateDefaultValues.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"

#ifndef WEBSSON_PARSER_DISABLE_IMPORT
#include "importManager.hpp"
#endif

using namespace std;
using namespace various;
using namespace webss;

static const char* ERROR_INPUT_DICTIONARY = "dictionary can only have key-values";
static const char* ERROR_INPUT_TUPLE = "tuple can only have concrete value-onlys or key-values";
static const char* ERROR_INPUT_NAMESPACE = "namespace can only have entity definitions";
static const char* ERROR_INPUT_DOCUMENT = "document can only have concrete value-onlys or key-values";
static const char* ERROR_OPTION = "expected option -v or --version";

static void useNamespace(EntityManager& ents, const Namespace& nspace)
{
	for (auto ent : nspace)
	{
		ent.removeNamespace();
		ents.addSafe(ent);
	}
}

static string getItPosition(const SmartIterator& it)
{
	return "[ln " + to_string(it.getLine()) + ", ch " + to_string(it.getCharCount()) + "]";
}

static string getItCurrentChar(const SmartIterator& it)
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
			throw runtime_error(WEBSSON_EXCEPTION("list can only contain values"));
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
		case Tag::IMPORT:
#ifdef WEBSSON_PARSER_DISABLE_IMPORT
			throw runtime_error(WEBSSON_EXCEPTION("this parser cannot import documents"));
#else
			{
				auto import = parseImport();
				const auto& link = import.getLink();
				const auto& headBody = ImportManager::getInstance().importDocument(link, filename);
				for (const auto& entPair : headBody.first)
					nspace.addSafe(entPair.second);
				break;
			}
#endif
		default:
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_INPUT_NAMESPACE));
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
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
		}
	});
}

Document Parser::parseDocument(various::SmartIterator it)
{
	setIterator(move(it));
	return parseDocument();
}

Document Parser::parseDocument(const std::istream& in)
{
	stringstream ss;
	ss << in.rdbuf();
	return parseDocument(SmartIterator(move(ss)));
}

Document Parser::parseDocument(various::SmartIterator it, string filename)
{
	setIterator(move(it));
	setFilename(move(filename));
	return parseDocument();
}

Document Parser::parseDocument(const std::istream& in, string filename)
{
	stringstream ss;
	ss << in.rdbuf();
	return parseDocument(SmartIterator(move(ss)), move(filename));
}

Document Parser::parseDocument()
{
	try
	{
		Document doc;
		if (containerEmpty())
			return doc;
		
		shared_ptr<string> docIdEnt(new string());
		do
		{
			switch (*tagit)
			{
			case Tag::ENTITY_ABSTRACT:
			{
				auto ent = parseAbstractEntity(Namespace::getEmptyInstance());
				ent.setDocId(docIdEnt);
				doc.addHead(ParamDocument::makeEntityAbstract(ent));
				ents.addSafe(move(ent));
				break;
			}
			case Tag::ENTITY_CONCRETE:
			{
				auto ent = parseConcreteEntity();
				ent.setDocId(docIdEnt);
				doc.addHead(ParamDocument::makeEntityConcrete(ent));
				ents.addSafe(move(ent));
				break;
			}
			case Tag::IMPORT:
#ifdef WEBSSON_PARSER_DISABLE_IMPORT
			throw runtime_error(WEBSSON_EXCEPTION("this parser cannot import documents"));
#else
			{
				auto import = parseImport();
				const auto& link = import.getLink();
				const auto& headBody = ImportManager::getInstance().importDocument(link, filename);
				for (const auto& entPair : headBody.first)
					ents.addSafe(entPair.second);
				for (const auto& keyValue : headBody.second)
					if (keyValue.first == "")
						doc.addImport(keyValue.second);
					else
						doc.addImportSafe(keyValue.first, keyValue.second);
				doc.addHead(move(import));
				break;
			}
#endif
			case Tag::EXPAND:
			{
				auto ent = parseExpandEntity(tagit, ents);
				auto contentType = ent.getContent().getType();
				if (contentType == WebssType::NAMESPACE)
				{
					auto param = ParamDocument::makeExpand(ent);
					const auto& nspace = param.getNamespace();
					useNamespace(ents, nspace);
					doc.addHead(move(param));
				}
				else if (contentType == WebssType::TUPLE || contentType == WebssType::TUPLE_TEXT)
					for (const auto& item : ent.getContent().getTuple().getOrderedKeyValues())
						item.first == nullptr ? doc.addBody(*item.second) : doc.addBodySafe(*item.first, *item.second);
				else
					throw runtime_error(WEBSSON_EXCEPTION("expand entity in document must be a namespace or tuple"));
				break;
			}
			case Tag::OPTION:
				parseOption();
				break;
			case Tag::EXPLICIT_NAME:
				parseExplicitKeyValue(
					CaseKeyValue{ doc.addBodySafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT));
				break;
			default:
				parseOtherValue(
					CaseKeyValue{ doc.addBodySafe(move(key), move(value)); },
					ErrorKeyOnly(ERROR_INPUT_DOCUMENT),
					CaseValueOnly{ doc.addBody(move(value)); },
					ErrorAbstractEntity(ERROR_INPUT_DOCUMENT));
				break;
			}
		} while (checkNextElement());
		*docIdEnt = move(docId);
		return doc;
	}
	catch (const exception& e)
	{
		throw runtime_error(string(getItPosition(getIt()) + ' ' + e.what() + getItCurrentChar(getIt())).c_str());
	}
}

Webss Parser::parseConcreteValueOnly(various::SmartIterator it)
{
	setIterator(move(it));
	return parseConcreteValueOnly();
}

Webss Parser::parseConcreteValueOnly()
{
	try { return parseValueOnly(); }
	catch (const exception& e) { throw runtime_error(WEBSSON_EXCEPTION(string(getItPosition(getIt()) + ' ' + e.what() + getItCurrentChar(getIt())).c_str())); }
}

#ifndef WEBSSON_PARSER_DISABLE_IMPORT
ImportedDocument Parser::parseImport()
{
	++tagit.getIt();
	auto link = parseStickyLineString(*this);
	return ImportedDocument(move(link));
}
#endif

static bool isOptionString(SmartIterator& it, const char* s)
{
	for (; *s != '\0'; ++s)
		if (++it != *s)
			return false;
	return true;
}

static string getOptionValue(Parser& self, SmartIterator& it)
{
	if (*it != CHAR_OPTION_ASSIGN_LINE_STRING)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	++it;
	return parseStickyLineStringOption(self);
}

static void checkMandatoryVersionValue(Parser& self, SmartIterator& it)
{
	if (!++it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
	auto value = getOptionValue(self, it);
	if (value == "*")
		throw runtime_error(WEBSSON_EXCEPTION("invalid mandatory WebSSON version"));
	if (value != "1.0.0")
		throw runtime_error(WEBSSON_EXCEPTION("this parser can only parse WebSSON version 1.0.0"));
}

static void skipOtherVersion(Parser& self, SmartIterator& it)
{
	//this function optimally seeks Tag::OPTION, so if the chars of that change, this must possibly be changed
start:
	if (!it)
		return;
startSkipLine:
	skipLine(++it);
getToOption:
	if (!it)
		return;
	if (*it != CHAR_OPTION)
		goto startSkipLine;
	if (!++it)
		return;
	if (*it != CHAR_OPTION)
		goto startSkipLine;
	
	//parse the option only if it corresponds to websson-version
	if (!skipLineJunk(++it))
		return;
	if (*it == '\n')
	{
		++it;
		goto getToOption;
	}
	if (*it != 'w')
		goto startSkipLine;
	if (!++it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
	if (*it == CHAR_OPTION_ASSIGN_LINE_STRING)
		; //do nothing
	else if (*it != 'e')
		goto startSkipLine;
	else if (!isOptionString(it, "bsson-version"))
		goto start;
	else if (!++it)
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
	else if (*it != CHAR_OPTION_ASSIGN_LINE_STRING)
		goto startSkipLine;
	else
	{
		auto value = getOptionValue(self, it);
		if (value != "*" && value != "1.0.0")
			goto start;
	}
}

void Parser::parseOption()
{
	auto& it = ++getItSafe();
	if (!skipLineJunk(it))
		return;
	if (*it == '\n')
	{
		++it;
		return;
	}
	
	if (*it == 'd')
	{
		if (!++it || (*it == 'o' && (!isOptionString(it, "cument-id") || !++it)))
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
		docId = getOptionValue(*this, it);
	}
	else if (*it == 'w')
	{
		if (!++it)
			throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
		else if (*it == 'e')
		{
			if (!isOptionString(it, "bsson-version") || !++it)
				throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
			if (*it == '-')
			{
				if (!isOptionString(it, "mandatory"))
	   				throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
				checkMandatoryVersionValue(*this, it);
				goto endParseOption;
			}
		}
		auto value = getOptionValue(*this, it);
		if (value != "*" && value != "1.0.0")
			skipOtherVersion(*this, it);
	}
	else if (*it == 'W')
		checkMandatoryVersionValue(*this, it);
	else	
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_OPTION));
endParseOption:
	if (!skipLineJunk(it))
		return;
	if (*it != '\n')
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
	++it;
}