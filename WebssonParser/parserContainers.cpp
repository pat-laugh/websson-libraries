//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

class ParserContainers : public Parser
{
private:
#define ExtraCasesEnum \
	ExtraCase(KeyType::KEYNAME, cont.addSafe(move(keyPair.first), cont.size())) \
	ExtraCase(KeyType::KEYWORD: case KeyType::VARIABLE: case KeyType::SCOPE: case KeyType::BLOCK_VALUE, throw runtime_error(ERROR_ANONYMOUS_KEY))

	PatternParseNameStart(Dictionary, ConType::DICTIONARY, cont.addSafe(move(keyPair.first), parseValue(it, ConType::DICTIONARY)),
		ExtraCase(KeyType::KEYWORD: case KeyType::KEYNAME: case KeyType::VARIABLE: case KeyType::SCOPE: case KeyType::BLOCK_VALUE, throw runtime_error(ERROR_ANONYMOUS_KEY)))
	PatternParseNameStart(List, ConType::LIST, throw runtime_error(ERROR_ADD_KEY_LIST), ExtraCasesAnonymous(ConType::LIST))
	PatternParseNameStart(Tuple, ConType::TUPLE, cont.addSafe(move(keyPair.first), parseValue(it, ConType::TUPLE)), ExtraCasesAnonymous(ConType::TUPLE))
	PatternParseNameStart(Document, ConType::DOCUMENT, cont.addSafe(move(keyPair.first), parseValue(it, ConType::DOCUMENT)), ExtraCasesAnonymous(ConType::DOCUMENT))
	PatternParseNameStart(Enum, ConType::LIST, throw runtime_error(ERROR_ADD_KEY_ENUM), ExtraCasesEnum)
public:
	Dictionary parseDictionary(It& it)
	{
		PatternParse(Dictionary cont; CheckEmpty(ConType::DICTIONARY, return cont), cont, ConType::DICTIONARY, CLOSE_DICTIONARY, ExtraCase(CHAR_CSTRING, addJsonKeyvalue(++it, cont)), CheckOtherValues(parseNameStartDictionary(it, cont), throw runtime_error(ERROR_ANONYMOUS_KEY)))
	}
	List parseList(It& it)
	{
		PatternParse(List cont; CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, ExtraCase(webss_CHAR_ANY_CONTAINER_CHAR_VALUE, cont.add(parseValue(it, ConType::LIST))), CheckOtherValues(parseNameStartList(it, cont), cont.add(parseNumber(it))))
	}
	Tuple parseTuple(It& it)
	{
		PatternParse(Tuple cont; CheckEmpty(ConType::TUPLE, return cont), cont, ConType::TUPLE, CLOSE_TUPLE, ExtraCase(webss_CHAR_ANY_CONTAINER_CHAR_VALUE, cont.add(parseValue(it, ConType::TUPLE))), CheckOtherValues(parseNameStartTuple(it, cont), cont.add(parseNumber(it))))
	}

	List parseListText(It& it)
	{
		PatternParse(List cont(true); CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, , cont.add(parseLineString(it, ConType::LIST));)
	}
	Tuple parseTupleText(It& it)
	{
		PatternParse(Tuple cont(true); CheckEmpty(ConType::TUPLE, return cont), cont, ConType::TUPLE, CLOSE_TUPLE, , cont.add(parseLineString(it, ConType::TUPLE));)
	}

#define CaseNamespace(Char, Func) ExtraCase(Char, checkMultiContainer(++it, [&]() { cont.add(Func); }))
#define ExtraCasesNamespace \
	CaseNamespace(CHAR_VARIABLE, parseVariable(it)) \
	CaseNamespace(CHAR_BLOCK, parseBlock(it))

	Namespace parseNamespace(It& it, const string& name)
	{
		PatternParse(Namespace cont(name); CheckEmpty(ConType::DICTIONARY, return cont), cont, ConType::DICTIONARY, CLOSE_DICTIONARY, ExtraCasesNamespace, throw runtime_error(ERROR_KEY_NAMESPACE);)
	}
	Enum parseEnum(It& it, const string& name)
	{
		PatternParse(Enum cont(name); CheckEmpty(ConType::LIST, return cont), cont, ConType::LIST, CLOSE_LIST, , CheckOtherValues(parseNameStartEnum(it, cont), throw runtime_error(ERROR_ANONYMOUS_KEY)))
	}

#define CON ConType::DOCUMENT
	Document parseDocument(It& it)
	{
#ifdef GET_LINE
		try
		{
#endif
			Document doc;

		documentHead:
			if (!skipJunk(it))
				return doc;

			switch (*it)
			{
			case CHAR_VARIABLE:
				checkMultiContainer(++it, [&]() { vars.add(parseVariable(it)); });
				break;
			case CHAR_BLOCK:
				checkMultiContainer(++it, [&]() { vars.add(parseBlock(it)); });
				break;
			case CHAR_OPTION:
#ifdef ALLOW_OPTION
				checkMultiContainer(++it, [&]() { parseOption(it); });
				break;
#else
				throw runtime_error("this parser does not have options");
#endif
			case CHAR_USING_NAMESPACE:
				checkMultiContainer(++it, [&]() { parseUsingNamespace(it); });
				break;
			case CHAR_IMPORT:
#ifdef ALLOW_OPTION
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
			do
			{
				switch (*it)
				{
				case webss_CHAR_ANY_CONTAINER_CHAR_VALUE:
					doc.add(parseValue(it, CON));
					break;
				default:
					if (checkOtherValues(it, [&]() { parseNameStartDocument(it, doc); }, [&]() { doc.add(parseNumber(it)); }))
						continue;
				}
				checkToNextElement(it, CON);
			} while (it);
			return doc;
#ifdef GET_LINE
		}
		catch (exception e)
		{
			string consoleClarification;
			if (it)
			{
				consoleClarification = consoleClarification + " \'" + *it + "\' ";
				switch (*it)
				{
				case '<': case '>':
					consoleClarification += "(angle bracket / chevron)";
					break;
				case '{': case '}':
					consoleClarification += "(curly bracket / brace)";
					break;
				case '(': case ')':
					consoleClarification += "(round bracket / parenthesis)";
					break;
				case '[': case ']':
					consoleClarification += "(square bracket)";
					break;
				case '~':
					consoleClarification += "(using namespace)";
					break;
				case '!':
					consoleClarification += "(block declaration)";
					break;
				case '@':
					consoleClarification += "(import)";
					break;
				case '#':
					consoleClarification += "(option)";
					break;
				case '&':
					consoleClarification += "(self)";
					break;
				case '?':
					consoleClarification += "(variable declaration)";
					break;
				case '.':
					consoleClarification += "(scope)";
					break;
				default:
					break;
				}
			}

			throw runtime_error(string("[ln " + to_string(it.getLine()) + ", ch " + to_string(it.getCharCol()) + "] " + e.what() + consoleClarification).c_str());
		}
#endif
	}
#undef CON
};

#define parserContainers static_cast<ParserContainers*>(this)

Dictionary Parser::parseDictionary(It& it) { return parserContainers->parseDictionary(it); }
Tuple Parser::parseTuple(It& it) { return parserContainers->parseTuple(it); }
List Parser::parseList(It& it) { return parserContainers->parseList(it); }

Tuple Parser::parseTupleText(It& it) { return parserContainers->parseTupleText(it); }
List Parser::parseListText(It& it) { return parserContainers->parseListText(it); }

Namespace Parser::parseNamespace(It& it, const string& name) { return parserContainers->parseNamespace(it, name); }
Enum Parser::parseEnum(It& it, const string& name) { return parserContainers->parseEnum(it, name); }

Document Parser::parseDocument(It& it) { return parserContainers->parseDocument(it); }

void Parser::checkMultiContainer(It& it, function<void()> func)
{
	if (*skipJunkToValid(it) != OPEN_DICTIONARY)
		func();
	else
	{
		++it;
		PatternParse(CheckEmpty(ConType::DICTIONARY, return), , ConType::DICTIONARY, CLOSE_DICTIONARY, , func();)
	}
}

Webss Parser::parseContainerText(It& it)
{
	switch (*it)
	{
	case OPEN_DICTIONARY:
		return parseDictionaryText(++it);
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

Block Parser::parseBlockValue(It& it, ConType con, const string& blockName)
{
	return Block(varsBlockId[blockName], checkIsValue(parseValueEqual(it, con)));
}

Block Parser::parseBlockValue(It& it, ConType con, const BasicVariable<BlockId>& blockId)
{
	return Block(blockId, checkIsValue(parseValueEqual(it, con)));
}