//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

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

void Deserializer::putFuncBodyBinary(StringBuilder& out, const FunctionHeadBinary::Tuple& params, const Tuple& data)
{
	Tuple::size_type i = 0;
	for (const auto& webss : data)
	{
		const auto& binary = params.at(i++);
		if (binary.sizeHead.getFlag() == ParamBinary::SizeHead::Flag::NONE)
			putBinary(out, binary, webss);
		else if (webss.t == WebssType::DEFAULT)
			out += '\x01';
		else
		{
			out += '\x00';
			if (binary.sizeHead.getFlag() == ParamBinary::SizeHead::Flag::SELF)
				putFuncBodyBinary(out, params, webss.getTuple());
			else
				putBinary(out, binary, webss);
		}
	}
}

void Deserializer::putBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data)
{
	const auto& sizeHead = bhead.sizeHead;
	if (!sizeHead.isFunctionHead())
	{
		putBinary(out, bhead, data, [&](const Webss& webss) { putBinaryElement(out, sizeHead, webss); });
		return;
	}

	const auto& params = bhead.sizeHead.getFunctionHead().getParameters();
	putBinary(out, bhead, data, [&](const Webss& webss) { putFuncBodyBinary(out, params, webss.getTuple()); });
}

void deserializeBitList(StringBuilder& out, const List& list)
{
	char c = 0;
	int shift = 0;
	for (const Webss& webss : list)
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

void Deserializer::putBinary(StringBuilder& out, const ParamBinary& bhead, const Webss& data, function<void(const Webss& webss)> func)
{
	if (bhead.sizeList.isOne())
	{
		func(data);
		return;
	}

	const List& list = data.getList();
	if (bhead.sizeList.isEmpty())
		writeBinarySize(out, list.size());

	if (bhead.sizeHead.isBool())
	{
		deserializeBitList(out, list);
		return;
	}

	for (const Webss& webss : list)
		func(webss);
}

void Deserializer::putBinaryElement(StringBuilder& out, const ParamBinary::SizeHead& bhead, const Webss& webss)
{
	if (bhead.isKeyword())
		switch (bhead.getKeyword())
		{
		case Keyword::BOOL:
			writeBytes(out, 1, reinterpret_cast<char*>(const_cast<bool*>(&webss.tBool)));
			return;
		case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8:
			writeBytes(out, bhead.getKeyword().getSize(), reinterpret_cast<char*>(const_cast<WebssInt*>(&webss.tInt)));
			return;
		case Keyword::DEC4: case Keyword::DEC8:
			writeBytes(out, bhead.getKeyword().getSize(), reinterpret_cast<char*>(const_cast<double*>(&webss.tDouble)));
			return;
		default:
			throw domain_error("");
		}
	else
	{
		if (bhead.isEmpty())
			writeBinarySize(out, webss.tString->length());
		out += webss.getString();
	}
}

void Deserializer::putBinarySizeHead(StringBuilder& out, const ParamBinary::SizeHead& bhead)
{
	using Type = ParamBinary::SizeHead::Type;
	out += OPEN_TUPLE;
	if (bhead.getFlag() == ParamBinary::SizeHead::Flag::SELF)
		out += CHAR_SELF;

	switch (bhead.getType())
	{
	case Type::EMPTY:
		break;
	case Type::KEYWORD:
		out += bhead.getKeyword().toString();
		break;
	case Type::NUMBER:
		out += to_string(bhead.size());
		break;
	case Type::FUNCTION_HEAD:
		putFheadBinary(out, bhead.getFunctionHead());
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		putEntityName(out, bhead.getEntityNumber());
		break;
	case Type::ENTITY_FUNCTION_HEAD:
		putEntityName(out, bhead.getEntityFunctionHead());
		break;
	default:
		throw domain_error("");
	}
	out += CLOSE_TUPLE;
}

void Deserializer::putBinarySizeList(StringBuilder& out, const ParamBinary::SizeList& blist)
{
	using Type = ParamBinary::SizeList::Type;
	if (blist.isOne())
		return;

	out += OPEN_LIST;
	switch (blist.getType())
	{
	case Type::EMPTY:
		break;
	case Type::NUMBER:
		out += to_string(blist.size());
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		putEntityName(out, blist.getEntity());
		break;
	default:
		throw domain_error("");
	}
	out += CLOSE_LIST;
}