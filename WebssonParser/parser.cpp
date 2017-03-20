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

GlobalParser::GlobalParser() : it(SmartIterator(string(""))) {}
GlobalParser::GlobalParser(SmartIterator&& it) : it(move(it)) {}
GlobalParser::GlobalParser(const std::istream& in) : it(SmartIterator(in)) {}
GlobalParser::GlobalParser(const std::stringstream& in) : it(SmartIterator(in)) {}
GlobalParser::GlobalParser(const std::string& in) : it(SmartIterator(in)) {}

GlobalParser& GlobalParser::setIterator(SmartIterator&& it)
{
	this->it = move(it);
	return *this;
}

GlobalParser& GlobalParser::setLanguage(Language lang)
{
	language = lang;
	separator = getLanguageSeparator(lang);
	return *this;
}

GlobalParser& GlobalParser::addEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
	return *this;
}

Document GlobalParser::parse() { return GlobalParser::Parser::parseDocument(*this); }