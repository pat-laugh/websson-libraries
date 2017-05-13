//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

namespace webss
{
	//char is '0' or '1'
	bool isDigitBin(char c);

	//char between '0' and '7'
	bool isDigitOct(char c);

	//char between '0' and '9'
	bool isDigitDec(char c);

	//returns true if char is a hexadecimal digit, else false
	bool isDigitHex(char c);

	//returns a decimal version of the binary digit
	//REQUIREMENT: c must be a binary digit
	int binToInt(char c);

	//returns a decimal version of the octal digit
	//REQUIREMENT: c must be a octal digit
	int octToInt(char c);

	//returns an int version of the char digit
	//REQUIREMENT: c must be a decimal digit
	int decToInt(char c);

	//returns a decimal version of the hexadecimal digit
	//REQUIREMENT: c must be a hexadecimal digit
	int hexToInt(char c);

	//returns a char version of the hexadecimal value
	//REQUIREMENT: i must be between 0 and 15
	char hexToChar(int i);

	//all ASCII control characters
	bool isControlAscii(char c);

	//all ASCII printable characters except space, digits and letters
	bool isSpecialAscii(char c);

	template <unsigned int n>
	struct power2 { enum : unsigned long long int { value = (unsigned long long int)2 << (n - 1) }; };

	template <>
	struct power2<0> { enum : unsigned long long int { value = 1 }; };
}