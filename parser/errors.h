//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	const char ERROR_EXPECTED[] = "expected character";
	const char ERROR_BINARY_SIZE_HEAD[] = "size of binary head must be a positive integer, binary template head or equivalent entity";
	const char ERROR_BINARY_SIZE_LIST[] = "size of binary list must be a positive integer or equivalent entity";

	const char ERROR_ADD_VALUE_DICTIONARY[] = "can't add value-only to dictionary";
	const char ERROR_ADD_KEY[] = "can't add key-value to ";
	const char ERROR_ADD_KEY_LIST[] = "can't add key-value to list";

	const char ERROR_UNEXPECTED[] = "unexpected character";
	const std::string ERROR_EXPECTED_CHARs1("expected character '");
	const std::string STRING_SINGLE_QUOTE("'");
#define webss_ERROR_EXPECTED_CHAR(x) ERROR_EXPECTED_CHARs1 + x + STRING_SINGLE_QUOTE

	const std::string ERROR_UNDEFINED_KEYNAMEs1("undefined entity: ");
#define webss_ERROR_UNDEFINED_KEYNAME(name) ERROR_UNDEFINED_KEYNAMEs1 + name

	const std::string ERROR_ENTITY_EXISTSs1("entity already exists: ");
#define webss_ERROR_ENTITY_EXISTS(name) ERROR_ENTITY_EXISTSs1 + name

	const char ERROR_EXPECTED_NUMBER[] = "expected number";
	const char ERROR_ANONYMOUS_KEY[] = "can't have anonymous key in dictionary, template head or enum";

	const char ERROR_KEY_NAMESPACE[] = "namespace members must be entities";
	const char ERROR_ADD_KEY_ENUM[] = "enums must have names without values";
}