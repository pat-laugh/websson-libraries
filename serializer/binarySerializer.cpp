//MIT License
//Copyright 2017 Patrick Laughrea
#include "binarySerializer.hpp"

#include <cassert>
#include <stdexcept>

#include "various/stringBuilder.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

StringBuilder& BinarySerializer::getOutputStream() { return bitshift == 0 ? main : temp; }

void BinarySerializer::flush()
{
	main += byteBlock;
	main += temp;
}

void BinarySerializer::flushAndReset()
{
	flush();
	byteBlock = 0;
	bitshift = 0;
}
	
BinarySerializer::BinarySerializer(StringBuilder& out) : main(out) {}

void BinarySerializer::putBit(int bit)
{
	byteBlock |= bit << bitshift;
	if (++bitshift == 8)
		flushAndReset();
}

void BinarySerializer::putBits(int numBits, int bits)
{
	int totalShift = bitshift + numBits;
	byteBlock |= bits << bitshift;
	if (totalShift < 8)
		bitshift = totalShift;
	else if (totalShift == 8)
		flushAndReset();
	else
	{
		//put remaining bits on a second byte
		flush();
		int bitshift2 = 8 - bitshift;
		byteBlock = bits >> bitshift2;
		bitshift = numBits - bitshift2;
	}
}

void BinarySerializer::putNumber(string::size_type number)
{
	auto& out = getOutputStream();
	if (number < power2<7>::value)
	{
		out += (char)number;
		return;
	}

#define POWER 7
	int utf7Bitshift;
	if (number < power2<POWER * 2>::value)
		utf7Bitshift = POWER;
	else if (number < power2<POWER * 3>::value)
		utf7Bitshift = POWER * 2;
	else if (number < power2<POWER * 4>::value)
		utf7Bitshift = POWER * 3;
	else if (number < power2<POWER * 5>::value)
		utf7Bitshift = POWER * 4;
	else if (number < power2<POWER * 6>::value)
		utf7Bitshift = POWER * 5;
	else if (number < power2<POWER * 7>::value)
		utf7Bitshift = POWER * 6;
	else if (number < power2<POWER * 8>::value)
		utf7Bitshift = POWER * 7;
	else if (number < power2<POWER * 9>::value)
		utf7Bitshift = POWER * 8;
	else //assumed max 64 bits
		utf7Bitshift = POWER * 9;

	do
		out += (char)(0x80 | (number >> utf7Bitshift));
	while ((utf7Bitshift -= POWER) > 0);

	out += (char)(0x7F & number);
#undef POWER
}

void BinarySerializer::putBytes(string::size_type num, char* value)
{
	auto& out = getOutputStream();
#ifdef WEBSSON_REVERSE_ENDIANNESS
	value += num;
	while (num-- > 0)
		out += *--value;
#else
	while (num-- > 0)
		out += *value++;
#endif
}

void BinarySerializer::putByte(char byte)
{
	getOutputStream() += byte;
}

void BinarySerializer::putString(const string& s)
{
	getOutputStream() += s;
}