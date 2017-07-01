//MIT License
//Copyright 2017 Patrick Laughrea
#include "unicode.hpp"

#include <cassert>
#include <limits>

#include "errors.hpp"
#include "utilsNumbers.hpp"
#include "utilsSweepers.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

const char* ERROR_EXPECTED_HEX = "expected hexadecimal digit";

//adds a unicode char to the StringBuilder
//REQUIREMENT: num must be a valid unicode character
void putUnicode(StringBuilder& str, unsigned int num)
{
	if (num < power2<7>::value)
	{
		str += (char)num;
		return;
	}

	//utf-8 is composed of the head and the tail
	//the head starts with a number of 1s, indicating the total length of the utf-8 char, followed by a 0 and then 5 to 1 bits
	//the tail is composed of chars always starting by two bits 1 and 0 and is composed after the head

	//the head
	//the shift at the left creates the numbers of bits telling the total length of the utf-8 char
	//there is a pattern in the number of bits the unicode number can take;
	//it is explicified with power2<x>::value, where x is the max number of bits and value is the max unicode value + 1
	//for values < power2<11>::value (2048), the utf-8 char is 2 bytes long, therefore the head must have 110x xxxx as bits
	//the remaining bits (in the case above, 5 bits) in the head must then be filled by taking the upper-most bits in the value.
	//To do that, the value is shifted by the number of bytes in the tail times 6 (the number of bits available in each byte
	//since the first two bits are occupied by 1 and 0, which signifies it is a tail byte).
	//In the case of values < power2<11>::value, the tail is 1 byte long (total == 2, - 1 for the head -> tail == 1), so it is shifted by 6
	int bitShift;
	if (num < power2<11>::value)
		str += (char)(0b11000000 | (num >> (bitShift = 6)));
	else if (num < power2<16>::value)
		str += (char)(0b11100000 | (num >> (bitShift = 12)));
	else if (num < power2<21>::value)
		str += (char)(0b11110000 | (num >> (bitShift = 18)));
	else if (num < power2<26>::value)
		str += (char)(0b11111000 | (num >> (bitShift = 24)));
	else //assumed max 31 bits
		str += (char)(0b11111100 | (num >> (bitShift = 30)));

	//the tail
	//if the bit shift was greater than 6, the other bytes are filled with decreasing bit shifts
	while ((bitShift -= 6) >= 0)
		str += (char)(0x80 | ((num >> bitShift) & 0x3F));
}

//reads a hex number with numDigits digits
unsigned int readHex(SmartIterator& it, int numDigits)
{
	if (!it || !isDigitHex(*it))
		throw runtime_error(ERROR_EXPECTED_HEX);

	int hex = hexToInt(*it);
	for (int i = 1; i < numDigits; ++i)
	{
		if (!(++it) || !isDigitHex(*it))
			throw runtime_error(ERROR_EXPECTED_HEX);
		hex = hex * (int)NumberBase::HEX + hexToInt(*it);
	}
	++it;
	return hex;
}

void webss::putEscapedHex(SmartIterator& it, StringBuilder& str)
{
	if (*it == 'x')
		putUnicode(str, readHex(++it, 2));
	else if (*it == 'u')
		putUnicode(str, readHex(++it, 4));
	else
	{
		if (it.peekEnd() || !isDigitHex(it.peek()))
			throw runtime_error(ERROR_EXPECTED_HEX);
		else if (it.peek() > '7')
			throw overflow_error("escaped char value too high");

		putUnicode(str, readHex(++it, 8));
	}
}
