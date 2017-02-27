//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "language.h"

#include <cassert>
#include <stdexcept>

#include "WebssonUtils/constants.h"

using namespace std;
using namespace webss;

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
		assert(false); throw domain_error("");
	}
}