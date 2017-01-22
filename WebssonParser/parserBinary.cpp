//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

#include <limits>

using namespace std;
using namespace webss;

Webss parseBinary(It& it, const ParamBinary& bhead);
void parseBitList(It& it, List& list, WebssBinarySize length);
Webss parseBinary(It& it, const ParamBinary& bhead, function<Webss()> func);
Webss parseBinaryElement(It& it, const ParamBinary::SizeHead& bhead);
Tuple parseBinaryFunction(It& it, const FunctionHeadBinary::Tuple& params);

WebssBinarySize checkBinarySize(WebssInt sizeInt);

void setDefaultValueBinary(Tuple& tuple, const FunctionHeadBinary::Tuple& params, FunctionHeadBinary::Tuple::size_type index)
{
	tuple[index] = Webss(params[index].sizeHead.defaultValue);
}

#define BINARY_DEFAULT_VALUE 1

void Parser::parseBinaryHead(It& it, FunctionHeadBinary& fhead)
{
	using Bhead = ParamBinary::SizeHead;
	using Blist = ParamBinary::SizeList;
	using Flag = Bhead::Flag;

	Flag flag;
	if (*skipJunkToValid(it) != CHAR_SELF)
		flag = Flag::NONE;
	else
	{
		flag = Flag::SELF;
		skipJunkToValid(++it);
	}

	Bhead bhead;
	Blist blist;
	if (*it == CLOSE_TUPLE)
	{
		new (&bhead) Bhead(Bhead::Type::EMPTY);
		new (&blist) Blist(Blist::Type::ONE);
	}
	else if (*it == OPEN_LIST)
	{
		new (&bhead) Bhead(Bhead::Type::EMPTY);
		new (&blist) Blist(parseBinarySizeList(++it));
	}
	else
	{
		if (isNameStart(*it))
		{
			auto nameType = parseNameType(it);
			if (nameType.type == NameType::KEYWORD)
				new (&bhead) Bhead(nameType.keyword);
			else if (nameType.type == NameType::ENTITY)
			{
				if (nameType.entity.getContent().getType() == WebssType::FUNCTION_HEAD_BINARY)
					new (&bhead) Bhead(checkEntFheadBinary(nameType.entity));
				else
					new (&bhead) Bhead(checkEntTypeBinarySize(nameType.entity));
			}
			else
				throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(nameType.name));
		}
		else if (isNumberStart(*it)) //line: 82, 77, 75
			new (&bhead) Bhead(checkBinarySize(parseNumber(it).getInt()));
		else if (*it == OPEN_FUNCTION)
		{
			auto headSwitch = parseFunctionHead(++it);
			if (!headSwitch.isBinary())
				throw runtime_error(ERROR_BINARY_SIZE_HEAD);
			new (&bhead) Bhead(move(headSwitch.fheadBinary));
		}

		if (*skipJunkToValid(it) != OPEN_LIST)
			new (&blist) Blist(Blist::Type::ONE);
		else
			new (&blist) Blist(parseBinarySizeList(++it));
	}

	bhead.flag = flag;
	skipJunkToValidCondition(it, [&]() { return *it == CLOSE_TUPLE; });
	parseOtherValue(skipJunkToValid(++it), ConType::FUNCTION_HEAD,
		CaseKeyValue
		{
			bhead.setDefaultValue(move(value));
			if (bhead.flag != Flag::SELF)
				bhead.flag = Flag::DEFAULT;
			fhead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		},
		CaseKeyOnly
		{
			if (bhead.flag == Flag::SELF)
				throw runtime_error("binary param declared with self must have a default value");
			fhead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		},
		ErrorValueOnly(ERROR_ANONYMOUS_KEY),
		ErrorAbstractEntity(ERROR_ANONYMOUS_KEY));
}

Tuple Parser::parseFunctionTupleBinary(It& it, const FunctionHeadBinary::Tuple& params)
{
	auto tuple = parseBinaryFunction(++it, params);
	if (it != CLOSE_TUPLE)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CLOSE_TUPLE));
	++it;
	return tuple;
}

//reads a number following UTF-7 encoding thing
WebssBinarySize readNumber(SmartIterator& it)
{
	WebssBinarySize num = 0;
	do
	{
		if (!it)
			throw runtime_error(ERROR_EXPECTED);
		num = (num << 7) | (0x7F & *it);
		if ((*it & 0x80) == 0)
			break;
	} while (true);
	++it;
	return num;
}

//reads num number of bytes and puts them in the char pointer passed as parameter
//if the end of it is reached before all the bytes have been read, an error is thrown
//advances it past the last byte read
//REQUIREMENT: the char pointer must point to sufficient memory space
void readBytes(SmartIterator& it, WebssBinarySize num, char* value)
{
	for (; num-- > 0; ++it)
	{
		if (!it)
			throw runtime_error(ERROR_EXPECTED);
		*value++ = *it;
	}
}

char readByte(SmartIterator& it)
{
	if (!it)
		throw runtime_error(ERROR_EXPECTED);
	char c = *it;
	++it;
	return c;
}

#define GET_BINARY_LENGTH(x) x.isEmpty() ? readNumber(it) : x.size()
Webss parseBinary(It& it, const ParamBinary& bhead)
{
	if (!bhead.sizeHead.isFunctionHead())
		return parseBinary(it, bhead, [&]() { return parseBinaryElement(it, bhead.sizeHead); });

	const auto& params = bhead.sizeHead.getFunctionHead().getParameters();
	return parseBinary(it, bhead, [&]() { return parseBinaryFunction(it, params); });
}

void parseBitList(It& it, List& list, WebssBinarySize length)
{
	char c;
	int shift = 0;
	c = readByte(it);
	while (length-- > 0)
	{
		if (shift == 8)
		{
			c = readByte(it);
			shift = 0;
		}

		list.add(Webss(((c >> shift) & 1) == 1)); //need explicit Webss, else converted to int (despite the ==)
		++shift;
	}
}

Webss parseBinary(It& it, const ParamBinary& bhead, function<Webss()> func)
{
	if (bhead.sizeList.isOne())
		return func();

	List list;
	auto length = GET_BINARY_LENGTH(bhead.sizeList);
	if (bhead.sizeHead.isBool())
		parseBitList(it, list, length);
	else
		while (length-- > 0)
			list.add(func());

	return list;
}

Webss parseBinaryElement(It& it, const ParamBinary::SizeHead& bhead)
{
	if (bhead.isKeyword())
	{
		WebssInt value = 0;
		switch (bhead.keyword)
		{
		case Keyword::BOOL:
			return Webss(readByte(it) != 0);
		case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8:
			readBytes(it, bhead.keyword.getSize(), reinterpret_cast<char*>(&value));
			return Webss(value);
		case Keyword::DEC4: case Keyword::DEC8:
			readBytes(it, bhead.keyword.getSize(), reinterpret_cast<char*>(&value));
			return Webss(static_cast<double>(value));
		default:
			throw domain_error("");
		}
	}

	auto length = GET_BINARY_LENGTH(bhead);
	string value;
	value.resize(length);
	readBytes(it, length, const_cast<char*>(value.data()));
	return Webss(move(value));
}
#undef GET_BINARY_LENGTH

Tuple parseBinaryFunction(It& it, const FunctionHeadBinary::Tuple& params)
{
	using Bhead = ParamBinary::SizeHead;
	Tuple tuple(params.getSharedKeys());
	for (Tuple::size_type i = 0; i < tuple.size(); ++i)
	{
		const auto& bhead = params[i];
		if (bhead.sizeHead.flag == Bhead::Flag::NONE)
			tuple[i] = parseBinary(it, bhead);
		else if (readByte(it) == BINARY_DEFAULT_VALUE)
			setDefaultValueBinary(tuple, params, i);
		else
			tuple[i] = bhead.sizeHead.flag == Bhead::Flag::DEFAULT ? parseBinary(it, bhead) : parseBinaryFunction(it, params);
	}
	return tuple;
}

ParamBinary::SizeList Parser::parseBinarySizeList(It& it)
{
	using Blist = ParamBinary::SizeList;
	using Type = Blist::Type;
	if (checkEmptyContainer(it, ConType::LIST))
		return Blist(Type::EMPTY);

	Blist blist;
	try
	{
		if (isNameStart(*it))
		{
			auto nameType = parseNameType(it);
			if (nameType.type != NameType::ENTITY)
				throw;
			new (&blist) Blist(checkEntTypeBinarySize(nameType.entity));
		}
		else if (isNumberStart(*it))
			new (&blist) Blist(checkBinarySize(parseNumber(it).getInt()));
		else
			throw;
	}
	catch (exception)
	{
		throw runtime_error("value in binary list must be void or a positive integer");
	}

	skipJunkToValidCondition(it, [&]() { return *it == CLOSE_LIST; });
	++it;
	return blist;
}

WebssBinarySize checkBinarySize(WebssInt sizeInt)
{
	if (sizeInt > numeric_limits<WebssBinarySize>::max())
		throw runtime_error("binary size is too big");
	else if (sizeInt < 0)
		throw runtime_error("binary size must be positive");
	return static_cast<WebssBinarySize>(sizeInt);
}

const BasicEntity<WebssBinarySize>& Parser::checkEntTypeBinarySize(const Entity& ent)
{
	const auto& name = ent.getName();
	if (!entsTypeBinarySize.hasEntity(name))
		try { entsTypeBinarySize.add(name, checkBinarySize(ent.getContent().getInt())); }
		catch (exception e) { throw runtime_error(e.what()); }
	return entsTypeBinarySize[name];
}

#undef BINARY_DEFAULT_VALUE