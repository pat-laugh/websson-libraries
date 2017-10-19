//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "various/StringBuilder.hpp"

namespace webss
{
	class BinarySerializer
	{
	private:
		various::StringBuilder& main;
		various::StringBuilder temp;
		char bitshift = 0;
		char byteBlock = 0;

		void checkBitshift();
		
		various::StringBuilder& getOutputStream();
		
		void flushByteBlock();
		void flushByteBlockAndReset();
		
	public:
		BinarySerializer(various::StringBuilder& out);

		//REQUIREMENT: bit must be 0 or 1
		void putBit(int bit);

		//numBits must be <= 8 and > 0
		//the bits must be the least significant bits; non-used bits must be 0
		void putBits(int numBits, int bits);

		//puts a number following UTF-7 encoding thing
		void putNumber(std::string::size_type number);

		//puts num number of bytes from the char pointer passed as parameter
		//REQUIREMENT: the char pointer must point to sufficient memory space
		void putBytes(std::string::size_type num, char* value);

		void putByte(char byte);

		void putString(const std::string& s);
	};
}