//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "conType.h"

#include <assert.h>
#include <stdexcept>

using namespace std;
using namespace webss;

ConType& ConType::operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
ConType& ConType::operator=(ConType o) { const_cast<Enum&>(this->e) = o.e; return *this; }

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

bool ConType::isStart(char c) const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return c == OPEN_DICTIONARY;
	case ConType::LIST:
		return c == OPEN_LIST;
	case ConType::TUPLE:
		return c == OPEN_TUPLE;
	case ConType::TEMPLATE_HEAD:
		return c == OPEN_TEMPLATE;
	default:
		return false;
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
