//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsUsers.h"

using namespace std;
using namespace webss;

void webss::addHexEscape(string& str, unsigned int num)
{
	str += '\\';
#define CHAR_BITS 4
	if (num < power2<8>::value)
	{
		str += 'x';
		str += hexToChar(num >> CHAR_BITS);
		str += hexToChar(num & 0x0F);
		return;
	}

	int bitShift;
	if (num < power2<16>::value)
	{
		bitShift = CHAR_BITS * 4;
		str += 'u';
	}
	else
	{
		bitShift = CHAR_BITS * 8;
		str += 'U';
	}

	while ((bitShift -= CHAR_BITS) >= 0)
		str += hexToChar((num >> bitShift) & 0x0F);
#undef CHAR_BITS
}

ParamBinary webss::makeBinary(Keyword keyword, WebssBinarySize sizeList)
{
	if (!keyword.isType())
		throw std::domain_error("invalid binary type: " + keyword.toString());

	ParamBinary::SizeHead bhead(keyword);
	if (sizeList == 0)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::EMPTY));
	else if (sizeList == 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::ONE));
	else if (sizeList > 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(sizeList));
	else
		throw std::domain_error(ERROR_BINARY_SIZE_LIST);
}

ParamBinary webss::makeBinaryDefault(Keyword keyword, Webss&& defaultValue, WebssBinarySize sizeList)
{
	if (!keyword.isType())
		throw std::domain_error("invalid binary type: " + keyword.toString());

	ParamBinary::SizeHead bhead(keyword);
	bhead.setFlag(ParamBinary::SizeHead::Flag::DEFAULT);
	bhead.setDefaultValue(Webss(std::move(defaultValue)));
	if (sizeList == 0)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::EMPTY));
	else if (sizeList == 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::ONE));
	else if (sizeList > 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(sizeList));
	else
		throw std::domain_error(ERROR_BINARY_SIZE_LIST);
}

ParamBinary webss::makeBinarySelf(Keyword keyword, Webss&& defaultValue, WebssBinarySize sizeList)
{
	if (!keyword.isType())
		throw std::domain_error("invalid binary type: " + keyword.toString());

	ParamBinary::SizeHead bhead(keyword);
	bhead.setFlag(ParamBinary::SizeHead::Flag::SELF);
	bhead.setDefaultValue(Webss(std::move(defaultValue)));
	if (sizeList == 0)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::EMPTY));
	else if (sizeList == 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(ParamBinary::SizeList::Type::ONE));
	else if (sizeList > 1)
		return ParamBinary(std::move(bhead), ParamBinary::SizeList(sizeList));
	else
		throw std::domain_error(ERROR_BINARY_SIZE_LIST);
}