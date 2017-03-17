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

void ParserBuilder::setLanguage(Language lang)
{
	language = lang;
	separator = getLanguageSeparator(lang);
}

void ParserBuilder::addGlobalEntity(string&& name, Webss&& value)
{
	ents.addGlobal(move(name), move(value));
}

