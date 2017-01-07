//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "keychars.h"

using namespace std;
using namespace webss;

bool webss::isKeyChar(char c)
{
	switch (c)
	{
	case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case OPEN_FUNCTION: case CHAR_COLON: case CHAR_EQUAL: case CHAR_CSTRING:
		return true;
	default:
		return false;
	}
}

KeyType webss::getKeyType(char c)
{
	switch (c)
	{
	case OPEN_DICTIONARY:
		return KeyType::DICTIONARY;
	case OPEN_LIST:
		return KeyType::LIST;
	case OPEN_TUPLE:
		return KeyType::TUPLE;
	case OPEN_FUNCTION:
		return KeyType::FUNCTION;
	case CHAR_COLON:
		return KeyType::COLON;
	case CHAR_EQUAL:
		return KeyType::EQUAL;
	case CHAR_CSTRING:
		return KeyType::CSTRING;
	case CHAR_SCOPE:
		return KeyType::SCOPE;
	default:
		throw domain_error(ERROR_UNDEFINED);
	}
}