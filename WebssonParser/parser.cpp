//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <cassert>

#include "WebssonUtils/constants.h"

using namespace std;
using namespace webss;

char getLanguageSeparator(Language lang)
{
	if (lang == Language::DEFAULT || lang == Language::EN)
		return CHAR_SEPARATOR;
	assert(lang == Language::INTL || lang == Language::FR);
	return ';';
}

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

void Parser::addGlobalEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
}

