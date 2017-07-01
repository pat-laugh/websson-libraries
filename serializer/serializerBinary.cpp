//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializer.hpp"

#include "utils/constants.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

//#define REVERSE_ENDIANNESS_WRITE

void putBin(StringBuilder& out, const ParamBin& param, const Webss& data);
void putBin(StringBuilder& out, const ParamBin& param, const Webss& data, function<void(const Webss& webss)> func);
void putBinElement(StringBuilder& out, const ParamBin::SizeHead& bhead, const Webss& webss);

//returns a string containing a number encoded in UTF-7 encoding thing
void writeBinSize(StringBuilder& out, WebssBinSize num)
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

void writeBytes(StringBuilder& out, WebssBinSize num, char* value)
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

void putTemplateBodyBin(StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple)
{
	assert(tuple.size() == params.size() && "size of binary tuple must match params");
	decltype(params.size()) i = 0;
	for (const auto& webss : tuple)
	{
		const auto& binary = params[i++];
		if (!binary.getSizeHead().hasDefaultValue())
		{
			assert(!binary.getSizeHead().isTheadSelf());
			putBin(out, binary, webss);
		}
		else if (webss.getTypeRaw() == WebssType::DEFAULT || webss.getTypeRaw() == WebssType::NONE)
			out += CHAR_BIN_DEFAULT_TRUE;
		else
		{
			out += CHAR_BIN_DEFAULT_FALSE;
			if (binary.getSizeHead().isTheadSelf())
				putTemplateBodyBin(out, params, webss.getTuple());
			else
				putBin(out, binary, webss);
		}
	}
}

void putBin(StringBuilder& out, const ParamBin& param, const Webss& data)
{
	const auto& sizeHead = param.getSizeHead();
	if (!sizeHead.isTheadBin())
		putBin(out, param, data, [&](const Webss& webss) { putBinElement(out, sizeHead, webss); });
	else
	{
		const auto& params = sizeHead.getThead().getParams();
		putBin(out, param, data, [&](const Webss& webss) { putTemplateBodyBin(out, params, webss.getTuple()); });
	}
}

void serializeBitList(StringBuilder& out, const List& list)
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
		if (webss.getBool())
			c |= 1 << shift;
		++shift;
	}
	out += c;
}

void putBin(StringBuilder& out, const ParamBin& param, const Webss& data, function<void(const Webss& webss)> func)
{
	if (param.getSizeList().isOne())
	{
		func(data);
		return;
	}

	const auto& list = data.getList();
	if (param.getSizeList().isEmpty())
		writeBinSize(out, list.size());

	if (param.getSizeHead().isBool())
		serializeBitList(out, list);
	else
		for (const auto& webss : list)
			func(webss);
}

void putBinElement(StringBuilder& out, const ParamBin::SizeHead& bhead, const Webss& webss)
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
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_BOOL);
			out += webss.getBoolRaw() ? 1 : 0;
			break;
		case Keyword::INT8:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			out += (char)webss.getIntRaw();
			break;
		case Keyword::INT16:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			writeBytes(out, 2, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT32:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			writeBytes(out, 4, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT64:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			writeBytes(out, 8, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::FLOAT:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_DOUBLE);
			tFloat = static_cast<float>(webss.getDoubleRaw());
			writeBytes(out, 4, reinterpret_cast<char*>(&tFloat));
			break;
		case Keyword::DOUBLE:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_DOUBLE);
			tDouble = webss.getDoubleRaw();
			writeBytes(out, 8, reinterpret_cast<char*>(&tDouble));
			break;
		default:
			assert(false);
		}
	}
	else
	{
		assert(webss.getTypeRaw() == WebssType::PRIMITIVE_STRING);
		const auto& s = webss.getStringRaw();
		if (bhead.isEmpty())
			writeBinSize(out, s.length());
#ifdef assert
		else
			assert(bhead.size() == s.length());
#endif
		out += s;
	}
}