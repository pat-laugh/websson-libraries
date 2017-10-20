//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializer.hpp"

#include "binarySerializer.hpp"
#include "utils/base64.hpp"
#include "utils/constants.hpp"

using namespace std;
using namespace various;
using namespace webss;

extern void putTemplateBodyBin(StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple, bool isEncoded);
static void putTemplateBodyBin(BinarySerializer& out, const TheadBin::Params& params, const Tuple& tuple);
static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data);
static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data, function<void(const Webss& webss)> func);
static void putBinElement(BinarySerializer& out, const ParamBin::SizeHead& bhead, const Webss& webss);

//entry point from serializer.cpp
void putTemplateBodyBin(StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple, bool isEncoded)
{
	if (isEncoded)
	{
		StringBuilder tempOut;
		BinarySerializer binSerializer(tempOut);
		putTemplateBodyBin(binSerializer, params, tuple);
		binSerializer.flush();
		SmartIterator it(tempOut.str());
		out += encodeBase64(it);
	}
	else
	{
		BinarySerializer binSerializer(out);
		putTemplateBodyBin(binSerializer, params, tuple);
		binSerializer.flush();
	}
}

static void putTemplateBodyBin(BinarySerializer& out, const TheadBin::Params& params, const Tuple& tuple)
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
			out.putBit(CHAR_BIN_DEFAULT_TRUE);
		else
		{
			out.putBit(CHAR_BIN_DEFAULT_FALSE);
			if (binary.getSizeHead().isTheadSelf())
				putTemplateBodyBin(out, params, webss.getTuple());
			else
				putBin(out, binary, webss);
		}
	}
}

static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data)
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

static void serializeBitArray(BinarySerializer& out, const List& list)
{
	char c = 0;
	int shift = 0;
	for (const auto& webss : list)
	{
		if (shift == 8)
		{
			out.putByte(c);
			shift = c = 0;
		}
		if (webss.getBool())
			c |= 1 << shift;
		++shift;
	}
	out.putByte(c);
}

static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data, function<void(const Webss& webss)> func)
{
	if (param.getSizeArray().isOne())
	{
		func(data);
		return;
	}

	const auto& list = data.getList();
	if (param.getSizeArray().isEmpty())
		out.putNumber(list.size());

	if (param.getSizeHead().isBool())
		serializeBitArray(out, list);
	else
		for (const auto& webss : list)
			func(webss);
}

static void putBinElement(BinarySerializer& out, const ParamBin::SizeHead& bhead, const Webss& webss)
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
			out.putBit(webss.getBoolRaw() ? 1 : 0);
			break;
		case Keyword::INT8:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			out.putByte((char)webss.getIntRaw());
			break;
		case Keyword::INT16:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			out.putBytes(2, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT32:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			out.putBytes(4, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::INT64:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			tInt = webss.getIntRaw();
			out.putBytes(8, reinterpret_cast<char*>(&tInt));
			break;
		case Keyword::FLOAT:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_DOUBLE);
			tFloat = static_cast<float>(webss.getDoubleRaw());
			out.putBytes(4, reinterpret_cast<char*>(&tFloat));
			break;
		case Keyword::DOUBLE:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_DOUBLE);
			tDouble = webss.getDoubleRaw();
			out.putBytes(8, reinterpret_cast<char*>(&tDouble));
			break;
		case Keyword::VARINT:
			assert(webss.getTypeRaw() == WebssType::PRIMITIVE_INT);
			out.putNumber(webss.getIntRaw());
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
			out.putNumber(s.length());
#ifndef NDEBUG
		else
			assert(bhead.size() == s.length());
#endif
		out.putString(s);
	}
}