//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "typeContainers.h"

#include <assert.h>

using namespace std;
using namespace webss;

string ConType::toString() const
{
	switch (e)
	{
	case ConType::DOCUMENT:
		return "document";
	case ConType::DICTIONARY:
		return "dictionary";
	case ConType::LIST:
		return "list";
	case ConType::TUPLE:
		return "tuple";
	case ConType::TEMPLATE_HEAD:
		return "template head";
	default:
		assert(false); throw domain_error("");
	}
}

bool ConType::isEnd(char c) const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return c == CLOSE_DICTIONARY;
	case ConType::LIST:
		return c == CLOSE_LIST;
	case ConType::TUPLE:
		return c == CLOSE_TUPLE;
	case ConType::TEMPLATE_HEAD:
		return c == CLOSE_TEMPLATE;
	default:
		return false;
	}
}

bool ConType::hasEndChar() const
{
	return e != ConType::DOCUMENT;
}