//MIT License
//Copyright 2017 Patrick Laughrea
#include "base64.hpp"

#include <cassert>
#include <stdexcept>

#include "constants.hpp"
#include "various/stringBuilder.hpp"

using namespace std;
using namespace various;
using namespace webss;

static void readBytes(SmartIterator& it, char bytes[4]);
static void writeBytes(char bytes[4]);

static const char* ERROR_EXPECTED = "expected character";

#define DECODE_ALL_0 '+'
#define DECODE_ALL_1 '-'
#define DECODE_NUMBER '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9'
#define DECODE_UPPER 'Q': case 'W': case 'F': case 'R': case 'K': case 'Y': case 'J': case 'I': case 'O': case 'P': \
	case 'A': case 'S': case 'D': case 'T': case 'G': case 'H': case 'E': case 'U': case 'L': \
	case 'Z': case 'X': case 'C': case 'V': case 'B': case 'N': case 'M'
#define DECODE_LOWER 'q': case 'w': case 'f': case 'r': case 'k': case 'y': case 'j': case 'i': case 'o': case 'p': \
	case 'a': case 's': case 'd': case 't': case 'g': case 'h': case 'e': case 'u': case 'l': \
	case 'z': case 'x': case 'c': case 'v': case 'b': case 'n': case 'm'
#define DECODE_CHAR_1 '_'
#define DECODE_CHAR_2 '='
#define DECODE_IGNORED ' ': case '\x7f': \
	case '\x00': case '\x01': case '\x02': case '\x03': case '\x04': case '\x05': case '\x06': case '\x07': case '\x08': case '\x09': case '\x0a': case '\x0b': case '\x0c': case '\x0d': case '\x0e': case '\x0f': \
	case '\x10': case '\x11': case '\x12': case '\x13': case '\x14': case '\x15': case '\x16': case '\x17': case '\x18': case '\x19': case '\x1a': case '\x1b': case '\x1c': case '\x1d': case '\x1e': case '\x1f'
#define PutThreeBytes(byte) { sb += byte; sb += byte; sb += byte; }

SmartIterator webss::decodeBase64(SmartIterator& it)
{
	StringBuilder sb;
	union
	{
		char bytes[4];
		int32_t bytesInt;
	};
decodeStart:
	if (!it)
		throw runtime_error(ERROR_EXPECTED);
	switch (*it)
	{
	case CHAR_END_TUPLE:
		return sb.str();
	case DECODE_ALL_0:
		PutThreeBytes('\0')
		goto decodeEnd;
	case DECODE_ALL_1:
		PutThreeBytes('\xff');
		goto decodeEnd;
	case DECODE_IGNORED:
		goto decodeEnd;
	default:
		break;
	}
	readBytes(it, bytes);
	//the 4 bytes: 00111111|00222222|00333333|00112233
	//will become: 11111111|22222222|33333333|00000000
	bytesInt |= (bytesInt & 0x30) << 26;
	bytesInt |= (bytesInt & 0x0c) << 20;
	bytesInt |= (bytesInt & 0x03) << 14;
	bytes[3] = 0;
	sb += bytes;
decodeEnd:
	++it;
	goto decodeStart;
}

static void readBytes(SmartIterator& it, char bytes[4])
{
	int i = 0;
startSwitch:
	switch (*it)
	{
	case DECODE_IGNORED:
		goto restart;
	case DECODE_NUMBER:
		bytes[i] = *it - '0';
		break;
	case DECODE_UPPER:
		bytes[i] = *it - 'A' + 10;
		break;
	case DECODE_LOWER:
		bytes[i] = *it - 'a' + 36;
		break;
	case DECODE_CHAR_1:
		bytes[i] = 62;
		break;
	case DECODE_CHAR_2:
		bytes[i] = 63;
		break;
	default:
		throw runtime_error("forbidden character in encoded binary");
	}
	if (++i == 4)
		return;
restart:
	if (!++it)
		throw runtime_error(ERROR_EXPECTED);
	goto startSwitch;
}

#define ENCODE_NUMBER 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9
#define ENCODE_UPPER 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: \
	case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29: \
	case 30: case 31: case 32: case 33: case 34: case 35
#define ENCODE_LOWER 36: case 37: case 38: case 39: \
	case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47: case 48: case 49: \
	case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57: case 58: case 59: \
	case 60: case 61
#define ENCODE_CHAR_1 62
#define ENCODE_CHAR_2 63

#ifdef BASE64_ENCODE_ALL_ZEROS
#ifdef BASE64_ENCODE_ALL_ONES
#define SpecialCheck(Ones, Cmd) { \
	if (bytesInt == 0) { sb += DECODE_ALL_0; Cmd; } \
	if (bytesInt == Ones) { sb += DECODE_ALL_1; Cmd; } }
#else
#define SpecialCheck(Ones, Cmd) { \
	if (bytesInt == 0) { sb += DECODE_ALL_0; Cmd; } }
#endif
#else //BASE64_ENCODE_ALL_ZEROS
#ifdef BASE64_ENCODE_ALL_ONES
#define SpecialCheck(Ones, Cmd) { \
	if (bytesInt == Ones) { sb += DECODE_ALL_1; Cmd; } }
#else
#define SpecialCheck(Ones, Cmd) ;
#endif
#endif //BASE64_ENCODE_ALL_ZEROS

string webss::encodeBase64(SmartIterator& it)
{
	StringBuilder sb;
	union
	{
		char bytes[5];
		int32_t bytesInt;
	};
	bytes[4] = 0;
encodeStart:
	if (!it)
		return sb;
	bytesInt = 0;
	bytes[0] = *it;
	if (!++it)
	{
		SpecialCheck(0xff000000, return sb)
		goto encodeBytes;
	}
	bytes[1] = *it;
	if (!++it)
	{
		SpecialCheck(0xffff0000, return sb)
		goto encodeBytes;
	}
	bytes[2] = *it;
	SpecialCheck(0xffffff00, ++it; goto encodeStart)
encodeBytes:
	//the 4 bytes: 11111111|22222222|33333333|00000000
	//will become: 00111111|00222222|00333333|00112233
	bytesInt |= (bytesInt & (0xc0 << 24)) >> 26;
	bytesInt |= (bytesInt & (0xc0 << 16)) >> 20;
	bytesInt |= (bytesInt & (0xc0 << 8)) >> 14;
	writeBytes(bytes);
	sb += bytes;
	++it;
	goto encodeStart;
}

static void writeBytes(char bytes[4])
{
	for (int i = 0; i < 4; ++i)
		switch (bytes[i])
		{
		case ENCODE_NUMBER:
			bytes[i] += '0';
			break;
		case ENCODE_UPPER:
			bytes[i] += 'A' - 10;
			break;
		case ENCODE_LOWER:
			bytes[i] += 'a' - 36;
			break;
		case ENCODE_CHAR_1:
			bytes[i] = DECODE_CHAR_1;
			break;
		case ENCODE_CHAR_2:
			bytes[i] = DECODE_CHAR_2;
			break;
		default:
			assert(false); throw domain_error("");
		}
}