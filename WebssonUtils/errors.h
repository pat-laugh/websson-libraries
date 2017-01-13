//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	const char ERROR_EXPECTED[] = "expected character";
	const char ERROR_UNDEFINED[] = "undefined behavior";
	const char ERROR_VALUE[] = "unexpected value";
	const char ERROR_BINARY_SIZE_HEAD[] = "size of binary head must be a positive integer, binary function head or equivalent variable";
	const char ERROR_BINARY_SIZE_LIST[] = "size of binary list must be a positive integer or equivalent variable";

	const char ERROR_ADD_VALUE_DICTIONARY[] = "can't add value-only to dictionary";
	const char ERROR_ADD_KEY[] = "can't add key-value to ";
	const char ERROR_ADD_KEY_LIST[] = "can't add key-value to list";

	const char ERROR_UNEXPECTED[] = "unexpected character";
	const std::string ERROR_EXPECTED_CHARs1("expected character '");
	const std::string STRING_SINGLE_QUOTE("'");
#define webss_ERROR_EXPECTED_CHAR(x) ERROR_EXPECTED_CHARs1 + x + STRING_SINGLE_QUOTE

	const std::string ERROR_UNDEFINED_KEYNAMEs1("undefined variable: ");
#define webss_ERROR_UNDEFINED_KEYNAME(name) ERROR_UNDEFINED_KEYNAMEs1 + name

	const std::string ERROR_VARIABLE_EXISTSs1("variable already exists: ");
#define webss_ERROR_VARIABLE_EXISTS(name) ERROR_VARIABLE_EXISTSs1 + name

	const char ERROR_EXPECTED_NUMBER[] = "expected number";
	const char ERROR_KEYWORD_KEY[] = "can't have keyword as key";
	const char ERROR_EMPTY_FUNCTION_HEAD[] = "function head can't be empty";
	const char ERROR_ANONYMOUS_KEY[] = "can't have anonymous key in dictionary, function head or enum";
	const char ERROR_CONTAINER_NOT_CLOSED[] = "container is not closed";
	const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";
	const char ERROR_DEREFERENCED_VARIABLE[] = "dereferenced element must be a variable of type dictionary";
	const char ERROR_FUNCTION_BODY[] = "expected function body";

	const char ERROR_KEY_NAMESPACE[] = "namespace members must be variables or blocks";
	const char ERROR_ADD_KEY_ENUM[] = "enums must have names without values";
}