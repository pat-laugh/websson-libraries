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
static void putBinKeyword(BinarySerializer& out, Keyword keyword, const Webss& webss);
template <bool putSize> static void putBinString(BinarySerializer& out, const string& s);

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
		const auto& param = params[i++];
		const auto& sizeHead = param.getSizeHead();
		if (!sizeHead.hasDefaultValue())
		{
			assert(!sizeHead.isTheadSelf() && webss.getTypeRaw() != WebssType::DEFAULT && webss.getTypeRaw() != WebssType::NONE);
			putBin(out, param, webss);
		}
		else if (webss.getTypeRaw() == WebssType::DEFAULT || webss.getTypeRaw() == WebssType::NONE)
			out.putBit(CHAR_BIN_DEFAULT_TRUE);
		else
		{
			out.putBit(CHAR_BIN_DEFAULT_FALSE);
			if (sizeHead.isTheadSelf())
				putTemplateBodyBin(out, params, webss.getTuple());
			else
				putBin(out, param, webss);
		}
	}
}

static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data)
{
	const auto& sizeHead = param.getSizeHead();
	if (sizeHead.isTheadBin())
		putBin(out, param, data, [&](const Webss& webss) { putTemplateBodyBin(out, sizeHead.getThead().getParams(), webss.getTuple()); });
	else
		putBin(out, param, data, [&](const Webss& webss) { putBinElement(out, sizeHead, webss); });
}

static void putBin(BinarySerializer& out, const ParamBin& param, const Webss& data, function<void(const Webss& webss)> func)
{
	if (param.getSizeArray().isOne())
		func(data);
	else
	{
		const auto& list = data.getListRaw();
		if (param.getSizeArray().isEmpty())
			out.putNumber(list.size());
		for (const auto& webss : list)
			func(webss);
	}
}

static void putBinElement(BinarySerializer& out, const ParamBin::SizeHead& bhead, const Webss& webss)
{
	using Type = ParamBin::SizeHead::Type;
	switch (bhead.getType())
	{
	case Type::KEYWORD:
		putBinKeyword(out, bhead.getKeyword(), webss);
		break;
	case Type::EMPTY: case Type::EMPTY_ENTITY_NUMBER:
		putBinString<true>(out, webss.getStringRaw());
		break;
	case Type::NUMBER: case Type::ENTITY_NUMBER:
		putBinString<false>(out, webss.getStringRaw());
		break;
	case Type::BITS: case Type::ENTITY_BITS:
		out.putBits(bhead.size(), webss.getIntRaw());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

static void putBinKeyword(BinarySerializer& out, Keyword keyword, const Webss& webss)
{
	union //this is put because rvalue cannot be taken for reinterpret_cast, so values are first stored here
	{
		WebssInt tInt;
		float tFloat;
		double tDouble;
	};
	
	switch (keyword)
	{
	case Keyword::BOOL:
		out.putBit(webss.getBoolRaw() ? 1 : 0);
		break;
	case Keyword::INT8:
		out.putByte((char)webss.getIntRaw());
		break;
	case Keyword::INT16:
		tInt = webss.getIntRaw();
		out.putBytes(2, reinterpret_cast<char*>(&tInt));
		break;
	case Keyword::INT32:
		tInt = webss.getIntRaw();
		out.putBytes(4, reinterpret_cast<char*>(&tInt));
		break;
	case Keyword::INT64:
		tInt = webss.getIntRaw();
		out.putBytes(8, reinterpret_cast<char*>(&tInt));
		break;
	case Keyword::FLOAT:
		tFloat = static_cast<float>(webss.getDoubleRaw());
		out.putBytes(4, reinterpret_cast<char*>(&tFloat));
		break;
	case Keyword::DOUBLE:
		tDouble = webss.getDoubleRaw();
		out.putBytes(8, reinterpret_cast<char*>(&tDouble));
		break;
	case Keyword::VARINT:
		out.putNumber(webss.getIntRaw());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

template <bool putSize> static void putBinString(BinarySerializer& out, const string& s)
{
	if (putSize)
		out.putNumber(s.length());
	out.putString(s);
}