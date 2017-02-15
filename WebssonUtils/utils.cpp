//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utils.h"

using namespace std;
using namespace webss;

bool webss::isDigitBin(char c)
{
	return c == '0' || c == '1';
}

bool webss::isDigit(char c)
{
	return c >= '0' && c <= '9';
}

bool webss::isDigitHex(char c)
{
	return isDigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int webss::binToInt(char c)
{
	return c - '0';
}

int webss::charToInt(char c)
{
	return c - '0';
}

int webss::hexToInt(char c)
{
	return c <= '9' ? c - '0' : 10 + c - (c <= 'F' ? 'A' : 'a');
}

char webss::hexToChar(int c)
{
	return (char)c + (c <= 9 ? '0' : 'A' - 10);
}

bool webss::isControlAscii(char c)
{
	return (unsigned char)c < ' ' || c == 127;
}

bool webss::isSpecialAscii(char c)
{
	switch (c)
	{
	case '`': case '~': case '!': case '@': case '#': case '$': case '%': case '^': case '&': case '*': case '(': case ')': case '-': case '_': case '=': case '+':
	case '[': case '{': case ']': case '}': case '\\': case '|':
	case ';': case ':': case '\'': case '"':
	case ',': case '<': case '.': case '>': case '/': case '?':
		return true;
	default:
		return false;
	}
}