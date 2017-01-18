//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

//returns a string containing a number encoded in UTF-7 encoding thing
string writeBinarySize(WebssBinarySize num)
{
	string out;
	if (num < power2<7>::value)
		return out + (char)num;

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

	return out + (char)(0x7F & num);
#undef POWER
}

//returns a string containing the specified number of bytes from the number in little endian
string writeBytes(WebssBinarySize num, char* value)
{
	string out;
	out.reserve(num);
#ifdef REVERSE_ENDIANNESS_WRITE
	value += num;
	while (num-- > 0)
		out += *--value;
#else
	while (num-- > 0)
		out += *value++;
#endif
	return out;
}

string webss::deserializeFunctionBodyBinary(const FunctionHeadBinary::Tuple& params, const Tuple& data)
{
	string out;
	Tuple::size_type i = 0;
	for (const auto& webss : data)
	{
		const auto& binary = params.at(i++);
		if (binary.sizeHead.flag == ParamBinary::SizeHead::Flag::NONE)
			out += deserializeBinary(binary, webss);
		else if (webss.t == WebssType::DEFAULT)
			out += '\x01';
		else if (binary.sizeHead.flag == ParamBinary::SizeHead::Flag::SELF)
			out += '\x00' + deserializeFunctionBodyBinary(params, webss.getTuple());
		else
			out += '\x00' + deserializeBinary(binary, webss);
	}
	return out;
}

string webss::deserializeBinary(const ParamBinary& bhead, const Webss& data)
{
	const auto& sizeHead = bhead.sizeHead;
	if (!sizeHead.isFunctionHead())
		return deserializeBinary(bhead, data, [&](const Webss& webss) { return deserializeBinaryElement(sizeHead, webss); });

	const auto& params = bhead.sizeHead.getFunctionHead().getParameters();
	return deserializeBinary(bhead, data, [&](const Webss& webss) { return deserializeFunctionBodyBinary(params, webss.getTuple()); });
}

string deserializeBitList(const List& list)
{
	string out;
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
	return out + c;
}

string webss::deserializeBinary(const ParamBinary& bhead, const Webss& data, function<string(const Webss& webss)> func)
{
	if (bhead.sizeList.isOne())
		return func(data);

	string out;
	const List& list = data.getList();
	if (bhead.sizeList.isEmpty())
		out += writeBinarySize(list.size());

	if (bhead.sizeHead.isBool())
		return out + deserializeBitList(list);

	for (const Webss& webss : list)
		out += func(webss);
	return out;
}

string webss::deserializeBinaryElement(const ParamBinary::SizeHead& bhead, const Webss& webss)
{
	if (bhead.isKeyword())
		switch (bhead.keyword)
		{
		case Keyword::BOOL:
			return writeBytes(1, reinterpret_cast<char*>(const_cast<bool*>(&webss.tBool)));
		case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8:
			return writeBytes(bhead.keyword.getSize(), reinterpret_cast<char*>(const_cast<WebssInt*>(&webss.tInt)));
		case Keyword::DEC4: case Keyword::DEC8:
			return writeBytes(bhead.keyword.getSize(), reinterpret_cast<char*>(const_cast<double*>(&webss.tDouble)));
		default:
			throw domain_error(ERROR_UNDEFINED);
		}
	else if (bhead.isEmpty())
		return writeBinarySize(webss.tString->length()) + webss.getString();
	else
		return webss.getString();
}

string webss::deserializeBinarySizeHead(const ParamBinary::SizeHead& bhead)
{
	using Type = ParamBinary::SizeHead::Type;
	string out;
	out += OPEN_TUPLE;
	if (bhead.flag == ParamBinary::SizeHead::Flag::SELF)
		out += CHAR_SELF;

	switch (bhead.t)
	{
	case Type::EMPTY:
		break;
	case Type::KEYWORD:
		out += bhead.keyword.toString();
		break;
	case Type::NUMBER:
		out += to_string(bhead.number);
		break;
	case Type::FUNCTION_HEAD:
		out += deserializeFunctionHeadBinary(*bhead.fhead);
		break;
	case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER: case Type::VARIABLE_FUNCTION_HEAD:
		out += bhead.getEntName();
		break;
	default:
		throw domain_error(ERROR_UNDEFINED);
	}
	return out + CLOSE_TUPLE;
}

string webss::deserializeBinarySizeList(const ParamBinary::SizeList& blist)
{
	using Type = ParamBinary::SizeList::Type;
	string out;
	out += OPEN_LIST;

	switch (blist.t)
	{
	case Type::EMPTY:
		break;
	case Type::ONE:
		return "";
	case Type::NUMBER:
		out += to_string(blist.number);
		break;
	case Type::EMPTY_VARIABLE_NUMBER: case Type::VARIABLE_NUMBER:
		out += blist.getEntName();
		break;
	default:
		throw domain_error(ERROR_UNDEFINED);
	}
	return out + CLOSE_LIST;
}