//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

Parser::Parser() : language(Language::DEFAULT), separator(CHAR_SEPARATOR) {}
Parser::Parser(Language lang) : language(lang), separator(getLanguageSeparator(lang)) {}

Folder Parser::parse(const string& in)
{
	SmartStringIterator it(in);
	return parseDocument(it);
}
Folder Parser::parse(istream& in)
{
	SmartStreamIterator it(in);
	return parseDocument(it);
}

void Parser::setLanguage(Language lang)
{
	language = lang;
	separator = getLanguageSeparator(lang);
}

void Parser::addVariable(string&& name, Webss&& value)
{
	vars.add(move(name), move(value));
}
void Parser::addBlock(string&& name, type_int value)
{
	auto nameCopy = name;
	varsBlockId.add(move(nameCopy), BlockId(move(name), value));
}

#define CON ConType::DOCUMENT
Folder Parser::parseDocument(It& it)
{
#ifdef GET_LINE
	try
	{
#endif
		Folder folder;

	documentHead:
		if (!skipJunk(it))
			return folder;

		switch (*it)
		{
		case CHAR_VARIABLE:
			checkMultiContainer(++it, [&]() { vars.add(parseVariable(it)); });
			break;
		case CHAR_BLOCK:
			checkMultiContainer(++it, [&]() { vars.add(parseBlock(it)); });
			break;
		case CHAR_OPTION:
			checkMultiContainer(++it, [&]() { readOption(it); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]() { getNamespace(it); });
			break;
		case CHAR_IMPORT: //TODO: allow import of files 16-12-26
#ifdef ALLOW_IMPORT
			checkMultiContainer(++it, [&]() { parseImport(it); });
			break;
#else
			throw runtime_error("this parser cannot import documents");
#endif
		default:
			goto documentBody;
		}
		checkToNextElement(it, CON);
		goto documentHead;

	documentBody:
		auto& doc = folder.getMainDocument();
		do
		{
			switch (*it)
			{
			case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_EQUAL: case CHAR_CSTRING:
				doc.add(parseValue(it, CON));
				break;
			default:
				if (checkOtherValues(it, [&]() { parseDocumentNameStart(it, doc); }, [&]() { doc.add(parseNumber(it)); }))
					continue;
			}
			checkToNextElement(it, CON);
		} while (it);
		return folder;
#ifdef GET_LINE
	}
	catch (exception e)
	{
		throw runtime_error(string("[ln " + to_string(it.getLine()) + "] " + e.what()).c_str());
	}
#endif
}

void Parser::parseDocumentNameStart(It& it, Document& doc)
{
	auto keyPair = parseKey(it);
	switch (keyPair.second)
	{
	case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE:
		doc.addSafe(move(keyPair.first), parseValue(it, CON));
		break;
	case KeyType::KEYWORD:
		doc.add(Keyword(keyPair.first));
		break;
	case KeyType::KEYNAME:
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(keyPair.first));
	case KeyType::VARIABLE:
		doc.add(checkIsValue(vars[keyPair.first]));
		break;
	case KeyType::SCOPE:
		doc.add(checkIsValue(parseScopedValue(it, keyPair.first)));
		break;
	case KeyType::BLOCK_VALUE:
		doc.add(parseBlockValue(it, CON, keyPair.first));
		break;
	default:
		throw runtime_error(webss_ERROR_UNEXPECTED);
	}
}
#undef CON

void Parser::readOption(It& it)
{
	if (*it != '-')
		throw runtime_error(webss_ERROR_UNEXPECTED);
	if (!(++it))
		throw runtime_error(ERROR_EXPECTED);

	if (*it != '-')
	{
		switch (*it)
		{
		case 'l': case 'L':
			if (++it != CHAR_COLON || !(++it))
				throw runtime_error(ERROR_EXPECTED);
			switch (*it)
			{
			case 'e': case 'E':
				if (++it && (*it == 'n' || *it == 'N'))
				{
					language = Language::EN;
					goto isEnd;
				}
				break;
			case 'f': case 'F':
				if (++it && (*it == 'r' || *it == 'R'))
				{
					language = Language::FR;
					goto isEnd;
				}
				break;
			case 'i': case 'I':
				if (++it && (*it == 'n' || *it == 'N') && ++it && (*it == 't' || *it == 'T'))
				{
					language = Language::INTL;
					goto isEnd;
				}
				break;
			case 'd': case 'D':
				if (++it && (*it == 'e' || *it == 'E') && ++it && (*it == 'f' || *it == 'F'))
				{
					language = Language::DEFAULT;
					goto isEnd;
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	throw runtime_error("could not read an option");
isEnd:
	separator = getLanguageSeparator(language);
	skipLine(++it);
}

void Parser::getNamespace(It& it)
{
	if (!isNameStart(*it))
		throw runtime_error(webss_ERROR_UNEXPECTED);

	auto name = parseName(it);
	if (!vars.hasVariable(name))
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(name));

	const auto& content = vars[name].getContent();
	if (!content.isDictionary())
		throw runtime_error("namespace name must be a variable of type dictionary");
	for (const auto& keyValue : content.getDictionary())
		vars.addSafe(keyValue.first, keyValue.second);
}

void Parser::checkMultiContainer(It& it, function<void()> func)
{
	skipJunkToValid(it);
	if (*it != OPEN_DICTIONARY)
	{
		func();
		return;
	}

#define CON ConType::DICTIONARY
	if (checkEmptyContainer(++it, CON))
		return;

	do
	{
		switch (*it)
		{
		case CLOSE_DICTIONARY:
			checkContainerEnd(it);
			return;
		default:
			if (checkSeparator(it))
				continue;
			func();
			checkToNextElement(it, CON);
		}
	} while (it);
	throw runtime_error(ERROR_CONTAINER_NOT_CLOSED);
#undef CON
}