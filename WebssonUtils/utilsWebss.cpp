//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsWebss.h"

#include <limits>

using namespace std;
using namespace webss;

bool webss::isNameStart(char c)
{
	return (c >= 'a' && c <= 'z') || (numeric_limits<char>::is_signed ? c < 0 : c > 127) || c == '_' || (c >= 'A' && c <= 'Z');
}

bool webss::isNameBody(char c)
{
	return isNameStart(c) || isDigit(c);
}

bool webss::isNumberStart(char c)
{
	return isDigit(c) || c == '-' || c == '+';
}

bool webss::isMagnitudeSeparator(char c)
{
	return c == 'e' || c == 'E' || c == 'm' || c == 'M';
}

bool webss::isJunk(char c)
{
	return c == ' ' || isControlAscii(c);
}

bool webss::isLineJunk(char c)
{
	return c != '\n' && isJunk(c);
}

bool webss::isEscapableChar(char c)
{
	switch (c)
	{
	case '0': case 'a': case 'b': case 'f': case 'n': case 'r': case 't': case 'v':
	case 's': case 'e': case 'x': case 'X': case 'u': case 'U':
		return true;
	default:
		return isSpecialAscii(c);
	}
}

bool webss::isMustEscapeChar(char c)
{
	return c == '?' || c == '\\' || isControlAscii(c);
}