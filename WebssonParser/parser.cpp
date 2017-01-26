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

void Parser::addGlobalEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
}

