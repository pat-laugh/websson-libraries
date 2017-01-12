#pragma once

#include "WebssonUtils/errors.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/typeContainers.h"

namespace webss
{
	//this is for when a named value (a key) is parsed; the char after determines its "type"
	enum class KeyType
	{
		DICTIONARY, LIST, TUPLE, FUNCTION, COLON, EQUAL, CSTRING, ALIAS,
		KEYWORD, KEYNAME, VARIABLE, SCOPE, BLOCK_VALUE
	};

	//returns whether or not c corresponds to a container start char, colon or equal
	bool isKeyChar(char c);

	//returns the KeyType corresponding to c
	KeyType getKeyType(char c);
}