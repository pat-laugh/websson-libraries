//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <assert.h>
#include <stdexcept>

#include "WebssonUtils/iterators.h"
#include "WebssonUtils/stringBuilder.h"

namespace webss
{
#define This BinaryIterator
	class This
	{
	private:
		SmartIterator& it;
		char bitshift = 8;
		char byteBlock;

		int getBitmask(int num)
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
			}
		}

		void checkBitshift()
		{
			if (bitshift == 8)
			{
				byteBlock = readByte();
				bitshift = 0;
			}
		}
	public:
		This(SmartIterator& it) : it(it) {}

		int readBit()
		{
			checkBitshift();
			int bit = (byteBlock >> bitshift) & 1;
			++bitshift;
			return bit;
		}

		//numBits must be <= 8 and > 0
		int readBits(int numBits)
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

		//reads a number following UTF-7 encoding thing
		std::string::size_type readNumber()
		{
			const int fullShift = 7, maxFullShifts = sizeof(std::string::size_type) / fullShift;
			std::string::size_type num = 0;
			int numShifts = 0;
		readLoop:
			readByte();
			num = (num << fullShift) | (0x7F & *it);
			if ((*it & 0x80) == 0)
				return num;
			if (++numShifts == maxFullShifts)
			{
				const int partShift = sizeof(std::string::size_type) % fullShift;
				const int partMask = 0xFF >> (8 - partShift);
				const int partEndMask = 0xFF ^ partMask;
				readByte();
				if ((*it & partEndMask) != 0)
					throw std::runtime_error("binary length is too great");
				num = (num << partShift) | (partMask & *it);
				return num;
			}
			goto readLoop;
		}

		//reads num number of bytes and puts them in the char pointer passed as parameter
		//if the end of it is reached before all the bytes have been read, an error is thrown
		//advances it past the last byte read
		//REQUIREMENT: the char pointer must point to sufficient memory space
		void readBytes(std::string::size_type num, char* value)
		{
			while (num-- > 0)
				*value++ = readByte();
		}

		inline char readByte()
		{
			if (!++it)
				throw std::runtime_error("expected character");
			return *it;
		}

		std::string readString(std::string::size_type num)
		{
			StringBuilder sb;
			while (num-- > 0)
				sb += readByte();
			return sb;
		}
	};
#undef This
}