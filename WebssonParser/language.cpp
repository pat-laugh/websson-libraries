//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "language.h"

using namespace std;
using namespace webss;

bool webss::isNumberEnd(char c, Language lang)
{
	return !isDecimalSeparator(c, lang) && !isMagnitudeSeparator(c);
}

bool webss::isDecimalSeparator(char c, Language lang)
{
	switch (lang)
	{
	case DEFAULT: case EN:
		return c == '.';
	case FR:
		return c == ',';
	case INTL:
		return c == '.' || c == ',';
	default:
		throw domain_error("");
	}
}

char webss::getLanguageSeparator(Language lang)
{
	switch (lang)
	{
	case DEFAULT: case EN:
		return CHAR_SEPARATOR;
	case INTL:
	case FR:
		return ';';
	default:
		throw domain_error("");
	}
}