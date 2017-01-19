//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

using namespace std;
using namespace webss;

Parser::Parser() : language(Language::DEFAULT), separator(CHAR_SEPARATOR) {}
Parser::Parser(Language lang) : language(lang), separator(getLanguageSeparator(lang)) {}

Document Parser::parse(const istream& in) { return parseDocument(in); }
Document Parser::parse(const stringstream& in) { return parseDocument(in); }
Document Parser::parse(const string& in) { return parseDocument(in); }

void Parser::setLanguage(Language lang)
{
	language = lang;
	separator = getLanguageSeparator(lang);
}

void Parser::addEntity(string&& name, Webss&& value)
{
	ents.add(move(name), move(value));
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
	while (++it && *it != '\n') //skip line
		;
}

