//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

Parser::Parser() : language(Language::DEFAULT), separator(CHAR_SEPARATOR) {}
Parser::Parser(Language lang) : language(lang), separator(getLanguageSeparator(lang)) {}

Document Parser::parse(const string& in)
{
	SmartStringIterator it(in);
	return parseDocument(it);
}
Document Parser::parse(istream& in)
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

void Parser::parseOption(It& it)
{
	if (*it != '-')
		throw runtime_error(ERROR_UNEXPECTED);
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

