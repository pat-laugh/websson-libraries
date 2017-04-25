//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "keywords.h"

#include <cassert>
#include <unordered_map>

using namespace std;
using namespace webss;

const unordered_map<string, Keyword> KEYWORDS = {
	{ "N", Keyword::KEY_NULL },
	{ "Nil", Keyword::KEY_NULL },
	{ "Null", Keyword::KEY_NULL },
	{ "nil", Keyword::KEY_NULL },
	{ "null", Keyword::KEY_NULL },

	{ "F", Keyword::KEY_FALSE },
	{ "False", Keyword::KEY_FALSE },
	{ "false", Keyword::KEY_FALSE },

	{ "T", Keyword::KEY_TRUE },
	{ "True", Keyword::KEY_TRUE },
	{ "true", Keyword::KEY_TRUE }
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
	case Keyword::INT8:
		return "byte";
	case Keyword::INT16:
		return "short";
	case Keyword::INT32:
		return "I";
	case Keyword::INT64:
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