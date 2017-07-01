//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "various/smartIterator.hpp"

namespace webss
{
	class BinIterator
	{
	private:
		various::SmartIterator& it;
		char bitshift = 8;
		char byteBlock;

		int getBitmask(int num);

		void checkBitshift();
	public:
		BinIterator(various::SmartIterator& it);

		int readBit();

		//numBits must be <= 8 and > 0
		int readBits(int numBits);

		//reads a number following UTF-7 encoding thing
		std::string::size_type readNumber();

		//reads num number of bytes and puts them in the char pointer passed as parameter
		//if the end of it is reached before all the bytes have been read, an error is thrown
		//advances it past the last byte read
		//REQUIREMENT: the char pointer must point to sufficient memory space
		void readBytes(std::string::size_type num, char* value);

		char readByte();

		std::string readString(std::string::size_type num);
	};
}