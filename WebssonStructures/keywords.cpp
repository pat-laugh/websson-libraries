//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "keywords.h"

#include <cassert>
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

	{ "float", Keyword::FLOAT },

	{ "D", Keyword::DOUBLE },
	{ "double", Keyword::DOUBLE },

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
	case Keyword::FLOAT:
		return "float";
	case Keyword::DOUBLE:
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
	case Keyword::BOOL: case Keyword::INT1:
		return 1;
	case Keyword::INT2:
		return 2;
	case Keyword::INT4: case Keyword::FLOAT:
		return 4;
	case Keyword::INT8: case Keyword::DOUBLE:
		return 8;
	case Keyword::STRING:
		return 0;
	default:
		assert(false && "can't get size of keyword "); throw domain_error("");
	}
}

bool Keyword::isType() const
{
	switch (e)
	{
	case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::FLOAT: case Keyword::DOUBLE: case Keyword::STRING:
		return true;
	default:
		return false;
	}
}