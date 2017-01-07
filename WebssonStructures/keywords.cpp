//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "keywords.h"

#include <map>

using namespace std;
using namespace webss;

const map<string, Keyword> KEYWORDS = {
	{ "N", Keyword::KEY_NULL },
	{ "null", Keyword::KEY_NULL },
	{ "nil", Keyword::KEY_NULL },

	{ "F", Keyword::KEY_FALSE },
	{ "false", Keyword::KEY_FALSE },

	{ "T", Keyword::KEY_TRUE },
	{ "true", Keyword::KEY_TRUE },

	{ "B", Keyword::BOOL },
	{ "bool", Keyword::BOOL },

	{ "byte", Keyword::INT1 },

	{ "short", Keyword::INT2 },

	{ "I", Keyword::INT4 },
	{ "int", Keyword::INT4 },

	{ "L", Keyword::INT8 },
	{ "long", Keyword::INT8 },

	{ "float", Keyword::DEC4 },

	{ "D", Keyword::DEC8 },
	{ "double", Keyword::DEC8 },

	{ "S", Keyword::STRING },
	{ "string", Keyword::STRING }
};

bool webss::isKeyword(const string& s)
{
	return KEYWORDS.find(s) != KEYWORDS.end();
}

Keyword::Keyword(string s) : e(KEYWORDS.find(s)->second) {}

string Keyword::toString() const
{
	switch (e)
	{
	case Keyword::KEY_NULL:
		return "N";
	case Keyword::KEY_FALSE:
		return "F";
	case Keyword::KEY_TRUE:
		return "T";
	case Keyword::BOOL:
		return "B";
	case Keyword::INT1:
		return "byte";
	case Keyword::INT2:
		return "short";
	case Keyword::INT4:
		return "I";
	case Keyword::INT8:
		return "L";
	case Keyword::DEC4:
		return "float";
	case Keyword::DEC8:
		return "D";
	case Keyword::STRING:
		return "S";
	default:
		return "unknown";
	}
}

int Keyword::getSize() const
{
	switch (e)
	{
	case Keyword::BOOL:
		return 1;
	case Keyword::INT1:
		return 1;
	case Keyword::INT2:
		return 2;
	case Keyword::INT4:
		return 4;
	case Keyword::INT8:
		return 8;
	case Keyword::DEC4:
		return 4;
	case Keyword::DEC8:
		return 8;
	default:
		throw domain_error("can't get size of keyword " + toString());
	}
}

bool Keyword::isType() const
{
	switch (e)
	{
	case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::DEC4: case Keyword::DEC8: case Keyword::STRING:
		return true;
	default:
		return false;
	}
}