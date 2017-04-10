//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "deserializer.h"

#include "WebssonUtils/constants.h"
#include "WebssonUtils/utils.h"

using namespace std;
using namespace webss;

//#define REVERSE_ENDIANNESS_WRITE

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

void putFuncBodyBinary(StringBuilder& out, const TemplateHeadBinary::Parameters& params, const Tuple& tuple)
{
	assert(tuple.size() == params.size() && "size of binary tuple must match params");
	decltype(params.size()) i = 0;
	for (const auto& webss : tuple)
	{
		const auto& binary = params[i++];
		if (!binary.getSizeHead().hasDefaultValue())
		{
			assert(!binary.getSizeHead().isSelf());
			putBinary(out, binary, webss);
		}
		else if (webss.getType() == WebssType::DEFAULT || webss.getType() == WebssType::NONE)
			out += CHAR_BINARY_DEFAULT_TRUE;
		else
		{
			out += CHAR_BINARY_DEFAULT_FALSE;
			if (binary.getSizeHead().isSelf())
				putFuncBodyBinary(out, params, webss.getTupleSafe());
			else
				putBinary(out, binary, webss);
		}
	}
}

void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data)
{
	const auto& sizeHead = param.getSizeHead();
	if (!sizeHead.isTemplateHead())
		putBinary(out, param, data, [&](const Webss& webss) { putBinaryElement(out, sizeHead, webss); });
	else
	{
		const auto& params = sizeHead.getTemplateHead().getParameters();
		putBinary(out, param, data, [&](const Webss& webss) { putFuncBodyBinary(out, params, webss.getTupleSafe()); });
	}
}

void deserializeBitList(StringBuilder& out, const List& list)
{
	char c = 0;
	int shift = 0;
	for (const auto& webss : list)
	{
		if (shift == 8)
		{
			out += c;
			shift = c = 0;
		}
		if (webss.getBoolSafe())
			c |= 1 << shift;
		++shift;
	}
	out += c;
}

void putBinary(StringBuilder& out, const ParamBinary& param, const Webss& data, function<void(const Webss& webss)> func)
{
	if (param.getSizeList().isOne())
	{
		func(data);
		return;
	}

	const auto& list = data.getListSafe();
	if (param.getSizeList().isEmpty())
		writeBinarySize(out, list.size());

	if (param.getSizeHead().isBool())
		deserializeBitList(out, list);
	else
		for (const auto& webss : list)
			func(webss);
}

void putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss)
{
	if (bhead.isKeyword())
	{
		union
		{
			WebssInt tInt;
			float tFloat;
			double tDouble;
		};

		switch (bhead.getKeyword())
		{
		case Keyword::BOOL:
			assert(webss.getType() == WebssType::PRIMITIVE_BOOL);
			out += webss.getBool() ? 1 : 0;
			break;
		case Keyword::INT8:
			assert(webss.getType() == WebssType::PRIMITIVE_INT);
			out += (char)webss.getInt();
			break;
		case Keyword::INT16:
			assert(webss.getType() == WebssType::PRIMITIVE_INT);
			tInt = webss.getInt();
			writeBytes(out, 2, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT32:
			assert(webss.getType() == WebssType::PRIMITIVE_INT);
			tInt = webss.getInt();
			writeBytes(out, 4, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT64:
			assert(webss.getType() == WebssType::PRIMITIVE_INT);
			tInt = webss.getInt();
			writeBytes(out, 8, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::FLOAT:
			assert(webss.getType() == WebssType::PRIMITIVE_DOUBLE);
			tFloat = static_cast<float>(webss.getDouble());
			writeBytes(out, 4, reinterpret_cast<char*>(&tFloat));
			break;
		case Keyword::DOUBLE:
			assert(webss.getType() == WebssType::PRIMITIVE_DOUBLE);
			tDouble = webss.getDouble();
			writeBytes(out, 8, reinterpret_cast<char*>(&tDouble));
			break;
		default:
			assert(false); throw domain_error("");
		}
	}
	else
	{
		assert(webss.getType() == WebssType::PRIMITIVE_STRING);
		const auto& s = webss.getString();
		if (bhead.isEmpty())
			writeBinarySize(out, s.length());
#ifdef assert
		else
			assert(bhead.size() == s.length());
#endif
		out += s;
	}
}