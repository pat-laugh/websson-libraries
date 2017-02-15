//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "WebssonUtils/utilsWebss.h"

namespace webss
{
	enum Language { DEFAULT, EN, FR, INTL };

	//returns the separator of the specified language
	char getLanguageSeparator(Language lang);

	//anything other than a decimal or magnitude separator can end a number
	bool isNumberEnd(char c, Language lang);

	//'.' or ','
	bool isDecimalSeparator(char c, Language lang);
}