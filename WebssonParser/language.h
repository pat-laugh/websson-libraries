#pragma once

#include "WebssonUtils/errors.h"
#include "WebssonUtils/endOfLine.h"
#include "WebssonUtils/utilsWebss.h"

namespace webss
{
	enum Language { DEFAULT, EN, FR, INTL };

	//return true if c is end of line, separator based on language or end of container, else false
	bool isLineEnd(char c, ConType con, Language lang);

	//returns the separator of the specified language
	char getLanguageSeparator(Language lang);

	//anything other than a decimal or magnitude separator can end a number
	bool isNumberEnd(char c, Language lang);

	//'.' or ','
	bool isDecimalSeparator(char c, Language lang);
}