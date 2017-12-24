//MIT License
//Copyright 2017 Patrick Laughrea
#include "conType.hpp"

#include <cassert>
#include <stdexcept>

#include "constants.hpp"

using namespace std;
using namespace webss;

ConType& ConType::operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
ConType& ConType::operator=(ConType o) { const_cast<Enum&>(this->e) = o.e; return *this; }

const char* ConType::toString() const
{
	switch (e)
	{
	default: assert(false);
	case ConType::DOCUMENT:
		return "document";
	case ConType::DICTIONARY:
		return "dictionary";
	case ConType::LIST:
		return "list";
	case ConType::TUPLE:
		return "tuple";
	case ConType::THEAD:
		return "template head";
	}
}

bool ConType::isStart(char c) const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return c == CHAR_START_DICTIONARY;
	case ConType::LIST:
		return c == CHAR_START_LIST;
	case ConType::TUPLE:
		return c == CHAR_START_TUPLE;
	case ConType::THEAD:
		return c == CHAR_START_TEMPLATE;
	default:
		return false;
	}
}

bool ConType::isEnd(char c) const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return c == CHAR_END_DICTIONARY;
	case ConType::LIST:
		return c == CHAR_END_LIST;
	case ConType::TUPLE:
		return c == CHAR_END_TUPLE;
	case ConType::THEAD:
		return c == CHAR_END_TEMPLATE;
	default:
		return false;
	}
}

bool ConType::hasEndChar() const
{
	return e != ConType::DOCUMENT;
}

char ConType::getStartChar() const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return CHAR_START_DICTIONARY;
	case ConType::LIST:
		return CHAR_START_LIST;
	case ConType::TUPLE:
		return CHAR_START_TUPLE;
	case ConType::THEAD:
		return CHAR_START_TEMPLATE;
	default:
		return 0;
	}
}

char ConType::getEndChar() const
{
	switch (e)
	{
	case ConType::DICTIONARY:
		return CHAR_END_DICTIONARY;
	case ConType::LIST:
		return CHAR_END_LIST;
	case ConType::TUPLE:
		return CHAR_END_TUPLE;
	case ConType::THEAD:
		return CHAR_END_TEMPLATE;
	default:
		return 0;
	}
}