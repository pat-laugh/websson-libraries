//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "typeContainers.h"

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
	case ConType::FUNCTION_HEAD:
		return "function head";
	default:
		return "unknown";
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
	case ConType::FUNCTION_HEAD:
		return c == CLOSE_FUNCTION;
	default:
		return false;
	}
}

bool ConType::hasEndChar() const
{
	return e != ConType::DOCUMENT;
}