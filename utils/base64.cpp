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

static int readBytes(SmartIterator& it, char bytes[4]);
static inline void parsePadding(SmartIterator& it, int bytesRead);

static const char* ERROR_EXPECTED = "expected character";
static const char* ERROR_UNEXPECTED = "forbidden character";
static const char* ERROR_PADDING = "forbidden character after padding";
static const char* ERROR_ONE_BYTE = "only one character available";

const static char CHAR_1 = '+', CHAR_2 = '/', PADDING = '=';
#define CaseDecodeNumber '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9'
#define CaseDecodeUpper 'Q': case 'W': case 'F': case 'R': case 'K': case 'Y': case 'J': case 'I': case 'O': case 'P': \
	case 'A': case 'S': case 'D': case 'T': case 'G': case 'H': case 'E': case 'U': case 'L': \
	case 'Z': case 'X': case 'C': case 'V': case 'B': case 'N': case 'M'
#define CaseDecodeLower 'q': case 'w': case 'f': case 'r': case 'k': case 'y': case 'j': case 'i': case 'o': case 'p': \
	case 'a': case 's': case 'd': case 't': case 'g': case 'h': case 'e': case 'u': case 'l': \
	case 'z': case 'x': case 'c': case 'v': case 'b': case 'n': case 'm'
#define CaseDecodeIgnored ' ': case '\x7f': \
	case '\x00': case '\x01': case '\x02': case '\x03': case '\x04': case '\x05': case '\x06': case '\x07': case '\x08': case '\x09': case '\x0a': case '\x0b': case '\x0c': case '\x0d': case '\x0e': case '\x0f': \
	case '\x10': case '\x11': case '\x12': case '\x13': case '\x14': case '\x15': case '\x16': case '\x17': case '\x18': case '\x19': case '\x1a': case '\x1b': case '\x1c': case '\x1d': case '\x1e': case '\x1f'
#define DecodeNumber(X) X - '0' + 52
#define DecodeUpper(X) X - 'A'
#define DecodeLower(X) X - 'a' + 26 
#define DecodeChar1(X) 62
#define DecodeChar2(X) 62

string webss::decodeBase64(SmartIterator& it)
{
	StringBuilder sb;
	char bytes[4];
	int bytesRead;
decodeStart:
	if (!it)
		throw runtime_error(ERROR_EXPECTED);
	switch (*it)
	{
	case CHAR_END_TUPLE:
		return sb.str();
	case CaseDecodeIgnored:
		++it;
		goto decodeStart;
	case PADDING:
		parsePadding(it, 0);
		goto decodeStart;
	default:
		break;
	}
	bytesRead = readBytes(it, bytes);
	if (bytesRead == 4)
		++it;
	else if (bytesRead == 1)
		throw runtime_error(ERROR_ONE_BYTE);
	else
		parsePadding(it, bytesRead);
	
	//the 4 bytes: 00111111|00112222|00222233|00333333
	//will become: 11111111|22222222|33333333|00000000
	bytes[0] = bytes[0] << 2 | bytes[1] >> 4;
	bytes[1] = bytes[1] << 4 | bytes[2] >> 2;
	bytes[2] = bytes[2] << 6 | bytes[3];
	
	for (int i = 0; i < bytesRead - 1; ++i)
		sb += bytes[i];
	goto decodeStart;
}

static int readBytes(SmartIterator& it, char bytes[4])
{
	int i = 0;
startSwitch:
	switch (*it)
	{
	case CaseDecodeIgnored:
		goto restart;
	case CaseDecodeNumber:
		bytes[i] = DecodeNumber(*it);
		break;
	case CaseDecodeUpper:
		bytes[i] = DecodeUpper(*it);
		break;
	case CaseDecodeLower:
		bytes[i] = DecodeLower(*it);
		break;
	case CHAR_1:
		bytes[i] = DecodeChar1(*it);
		break;
	case CHAR_2:
		bytes[i] = DecodeChar2(*it);
		break;
	case CHAR_END_TUPLE: case PADDING:
		return i;
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
	if (++i == 4)
		return i;
restart:
	if (!++it)
		throw runtime_error(ERROR_EXPECTED);
	goto startSwitch;
}

static inline void parsePadding(SmartIterator& it, int bytesRead)
{
	assert(bytesRead != 1 && bytesRead < 4);
	for (; bytesRead < 4 && it; ++it)
		switch (*it)
		{
		case CHAR_END_TUPLE:
			return;
		case CaseDecodeIgnored:
			break;
		case PADDING:
			++bytesRead;
			break;
		default:
			throw runtime_error(ERROR_PADDING);
		}
}

static char ENCODED_BYTES[] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

string webss::encodeBase64(SmartIterator& it)
{
	StringBuilder sb;
	union
	{
		unsigned char bytes[4];
		uint32_t bytesInt;
	};
	int numBytesOut = 4;
encodeStart:
	if (!it)
		return sb;
	
	bytes[0] = *it;
	if (!++it)
	{
		numBytesOut = 2;
		goto encodeBytes;
	}
	
	bytes[1] = *it;
	if (!++it)
	{
		numBytesOut = 3;
		goto encodeBytes;
	}
	
	bytes[3] = *it;
encodeBytes:
	//the 4 bytes: 11111111|22222222|xxxxxxxx|33333333
	//will become: 00111111|00112222|00222233|00333333
	bytes[2] = bytes[3] >> 6 | bytes[1] << 2;
	bytes[1] = bytes[1] >> 4 | bytes[0] << 4;
	bytes[0] = bytes[0] >> 2;
	bytesInt &= 0x3f3f3f3f;
	
	for (int i = 0; i < numBytesOut; ++i)
		sb += ENCODED_BYTES[bytes[i]];
	if (numBytesOut < 4)
	{
#ifdef WEBSSON_BASE64_PADDING
		while (numBytesOut++ < 4)
			sb += PADDING;
#endif
		return sb;
	}
	++it;
	goto encodeStart;
}