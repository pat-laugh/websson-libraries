//MIT License
//Copyright 2017-2018 Patrick Laughrea
#include "keywords.hpp"

#include <cassert>
#include <unordered_map>

using namespace std;
using namespace webss;

const Keyword::Enum Keyword::KEY_NULL;
const Keyword::Enum Keyword::KEY_FALSE;
const Keyword::Enum Keyword::KEY_TRUE;

static const unordered_map<string, Keyword> KEYWORDS = {
	{ "N", Keyword::KEY_NULL },
	{ "Nil", Keyword::KEY_NULL },
	{ "None", Keyword::KEY_NULL },
	{ "Null", Keyword::KEY_NULL },
	{ "nil", Keyword::KEY_NULL },
	{ "none", Keyword::KEY_NULL },
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

Keyword::Keyword(const string& s) : e(KEYWORDS.find(s)->second) {}

const char* Keyword::toString() const
{
	switch (e)
	{
	default: assert(false);
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
	case Keyword::VARINT:
		return "varint";
	}
}

Keyword& Keyword::operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
Keyword& Keyword::operator=(Keyword o) { const_cast<Enum&>(this->e) = o.e; return *this; }