//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

//utils for users

#include "utilsParser.h"
#include "WebssonUtils/utilsWebss.h"

namespace webss
{
	//adds '\\' followed by x, u or U followed by 2, 4 or 8 hex digits
	//REQUIREMENT: num must be a valid unicode character representable in max 8 hex digits
	void addHexEscape(std::string& str, unsigned int num);

	ParamBinary makeBinary(Keyword keyword, WebssBinarySize sizeList = 1);

	ParamBinary makeBinaryDefault(Keyword keyword, Webss&& defaultValue, WebssBinarySize sizeList = 1);

	ParamBinary makeBinarySelf(Keyword keyword, Webss&& defaultValue, WebssBinarySize sizeList = 1);
}