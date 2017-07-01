//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsWebss.hpp"

#include <various/utils.hpp>

using namespace various;
using namespace webss;

bool webss::isNameStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (unsigned char)c > 127;
}

bool webss::isNameBody(char c)
{
	return isNameStart(c) || isDigitDec(c);
}

bool webss::isNameSeparator(char c)
{
	return c == '_' || c == '-';
}

bool webss::isNumberStart(char c)
{
	return isDigitDec(c) || c == '-' || c == '+';
}

bool webss::isJunk(char c)
{
	return c == ' ' || isControlAscii(c);
}

bool webss::isLineJunk(char c)
{
	return c != '\n' && isJunk(c);
}
