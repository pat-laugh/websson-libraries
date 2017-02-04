//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data);
void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data, function<void(const Webss& webss)> func);
void putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss);

//returns a string containing a number encoded in UTF-7 encoding thing
void writeBinarySize(StringBuilder& out, WebssBinarySize num)
{
	if (num < power2<7>::value)
	{
		out += (char)num;
		return;
	}

#define POWER 7
	int bitShift;
	if (num < power2<POWER * 2>::value)
		bitShift = POWER;
	else if (num < power2<POWER * 3>::value)
		bitShift = POWER * 2;
	else if (num < power2<POWER * 4>::value)
		bitShift = POWER * 3;
	else if (num < power2<POWER * 5>::value)
		bitShift = POWER * 4;
	else if (num < power2<POWER * 6>::value)
		bitShift = POWER * 5;
	else if (num < power2<POWER * 7>::value)
		bitShift = POWER * 6;
	else if (num < power2<POWER * 8>::value)
		bitShift = POWER * 7;
	else if (num < power2<POWER * 9>::value)
		bitShift = POWER * 8;
	else //assumed max 64 bits
		bitShift = POWER * 9;

	do
		out += (char)(0x80 | (num >> bitShift));
	while ((bitShift -= POWER) > 0);

	out += (char)(0x7F & num);
#undef POWER
}

//returns a string containing the specified number of bytes from the number in little endian
void writeBytes(StringBuilder& out, WebssBinarySize num, char* value)
{
#ifdef REVERSE_ENDIANNESS_WRITE
	value += num;
	while (num-- > 0)
		out += *--value;
#else
	while (num-- > 0)
		out += *value++;
#endif
}

void putFuncBodyBinary2(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Tuple& tuple)
{
	assert(tuple.size() == params.size() && "size of binary tuple must match params");
	decltype(params.size()) i = 0;
	for (auto&& webss : tuple)
	{
		auto&& binary = params[i++];
		if (!binary.sizeHead.hasDefaultValue())
		{
			assert(!binary.sizeHead.isSelf());
			putBinary(out, binary, webss);
		}
		else if (webss.type == WebssType::DEFAULT || webss.type == WebssType::NONE)
			out += CHAR_BINARY_DEFAULT_TRUE;
		else
		{
			out += CHAR_BINARY_DEFAULT_FALSE;
			if (binary.sizeHead.isSelf())
				putFuncBodyBinary2(out, params, webss.getTuple());
			else
				putBinary(out, binary, webss);
		}
	}
}

void Deserializer::putFuncBodyBinary(StringBuilder& out, const FunctionHeadBinary::Parameters& params, const Tuple& tuple)
{
	putFuncBodyBinary2(out, params, tuple);
}

void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data)
{
	auto&& sizeHead = param.sizeHead;
	if (!sizeHead.isFunctionHead())
		putBinary(out, param, data, [&](const Webss& webss) { putBinaryElement(out, sizeHead, webss); });
	else
	{
		auto&& params = sizeHead.getFunctionHead().getParameters();
		putBinary(out, param, data, [&](const Webss& webss) { putFuncBodyBinary2(out, params, webss.getTuple()); });
	}
}

void deserializeBitList(StringBuilder& out, const List& list)
{
	char c = 0;
	int shift = 0;
	for (auto&& webss : list)
	{
		if (shift == 8)
		{
			out += c;
			shift = c = 0;
		}
		if (webss.getBool())
			c |= 1 << shift;
		++shift;
	}
	out += c;
}

void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data, function<void(const Webss& webss)> func)
{
	if (param.sizeList.isOne())
	{
		func(data);
		return;
	}

	auto&& list = data.getList();
	if (param.sizeList.isEmpty())
		writeBinarySize(out, list.size());

	if (param.sizeHead.isBool())
		deserializeBitList(out, list);
	else
		for (auto&& webss : list)
			func(webss);
}

void putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss)
{
	if (bhead.isKeyword())
		switch (bhead.getKeyword())
		{
		case Keyword::BOOL:
			assert(webss.type == WebssType::PRIMITIVE_BOOL);
			out += webss.tBool ? 1 : 0;
			break;
		case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8:
			assert(webss.type == WebssType::PRIMITIVE_INT);
			writeBytes(out, bhead.getKeyword().getSize(), reinterpret_cast<char*>(const_cast<WebssInt*>(&webss.tInt)));
			break;
		case Keyword::DEC4:
			assert(webss.type == WebssType::PRIMITIVE_DOUBLE);
			float f = static_cast<float>(webss.tDouble);
			writeBytes(out, sizeof(float), reinterpret_cast<char*>(&f));
			break;
		case Keyword::DEC8:
			assert(webss.type == WebssType::PRIMITIVE_DOUBLE);
			writeBytes(out, sizeof(double), reinterpret_cast<char*>(const_cast<double*>(&webss.tDouble)));
			break;
		default:
			assert(false); throw domain_error("");
		}
	else
	{
		assert(webss.type == WebssType::PRIMITIVE_STRING);
		const auto& s = *webss.tString;
		if (bhead.isEmpty())
			writeBinarySize(out, s.length());
		else
			assert(bhead.size() == s.length());
		out += s;
	}
}