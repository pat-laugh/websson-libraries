//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	const char ERROR_EXPECTED[] = "expected character";
	const char ERROR_BINARY_SIZE_HEAD[] = "size of binary head must be a positive integer, binary template head or equivalent entity";

	const char ERROR_UNEXPECTED[] = "unexpected character";

	std::string ERROR_UNDEFINED_KEYNAME(const std::string& name) { return "undefined entity: " + name; }

	const char ERROR_EXPECTED_NUMBER[] = "expected number";
	const char ERROR_ANONYMOUS_KEY[] = "can't have anonymous key in dictionary, template head or enum";
}