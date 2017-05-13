//MIT License
//Copyright 2017 Patrick Laughrea
#include "binaryIterator.hpp"

#include <cassert>
#include <stdexcept>

#include "utils/stringBuilder.hpp"

using namespace std;
using namespace webss;

int BinaryIterator::getBitmask(int num)
{
	switch (num)
	{
	case 1: return 0b00000001;
	case 2: return 0b00000011;
	case 3: return 0b00000111;
	case 4: return 0b00001111;
	case 5: return 0b00011111;
	case 6: return 0b00111111;
	case 7: return 0b01111111;
	case 8: return 0b11111111;
#ifdef assert
	default:
		assert(false);
		throw domain_error("");
#endif
	}
}

void BinaryIterator::checkBitshift()
{
	if (bitshift == 8)
	{
		byteBlock = readByte();
		bitshift = 0;
	}
}
	
BinaryIterator::BinaryIterator(SmartIterator& it) : it(it) {}

int BinaryIterator::readBit()
{
	checkBitshift();
	int bit = (byteBlock >> bitshift) & 1;
	++bitshift;
	return bit;
}

int BinaryIterator::readBits(int numBits)
{
	checkBitshift();

	if (bitshift + numBits <= 8)
	{
		int bits = (byteBlock >> bitshift) & getBitmask(numBits);
		bitshift += numBits;
		return bits;
	}

	//the number is spread on two bytes
	int numBitsPart1 = 8 - bitshift, numBitsPart2 = numBits - numBitsPart1;
	int bits = (byteBlock >> bitshift) & getBitmask(numBitsPart1);
	byteBlock = readByte();
	bits <<= numBitsPart2; //make space for part 2
	bits |= (byteBlock & getBitmask(numBitsPart2));
	bitshift = numBitsPart2;
	return bits;
}

string::size_type BinaryIterator::readNumber()
{
	const int fullShift = 7, maxFullShifts = sizeof(string::size_type) / fullShift;
	string::size_type num = 0;
	int numShifts = 0;
readLoop:
	readByte();
	num = (num << fullShift) | (0x7F & *it);
	if ((*it & 0x80) == 0)
		return num;
	if (++numShifts == maxFullShifts)
	{
		const int partShift = sizeof(string::size_type) % fullShift;
		const int partMask = 0xFF >> (8 - partShift);
		const int partEndMask = 0xFF ^ partMask;
		readByte();
		if ((*it & partEndMask) != 0)
			throw runtime_error("binary length is too great");
		num = (num << partShift) | (partMask & *it);
		return num;
	}
	goto readLoop;
}

void BinaryIterator::readBytes(string::size_type num, char* value)
{
	while (num-- > 0)
		*value++ = readByte();
}

char BinaryIterator::readByte()
{
	if (!++it)
		throw runtime_error("expected character");
	return *it;
}

string BinaryIterator::readString(string::size_type num)
{
	StringBuilder sb;
	while (num-- > 0)
		sb += readByte();
	return sb;
}