//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsWebss.h"

#include <limits>

using namespace std;
using namespace webss;

bool webss::isNameStart(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || (unsigned char)c > 127;
}

bool webss::isNameBody(char c)
{
	return isNameStart(c) || isDigit(c);
}

bool webss::isNumberStart(char c)
{
	return isDigit(c) || c == '-' || c == '+';
}

bool webss::isBaseSeparator(char c)
{
	return c == 'e' || c == 'E' || c == 'p' || c == 'P';
}

bool webss::isJunk(char c)
{
	return c == ' ' || isControlAscii(c);
}

bool webss::isLineJunk(char c)
{
	return c != '\n' && isJunk(c);
}