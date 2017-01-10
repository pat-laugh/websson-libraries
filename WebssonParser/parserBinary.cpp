//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"

#include <limits>

using namespace std;
using namespace webss;

Webss parseBinary(It& it, const ParamBinary& bhead);
void parseBitList(It& it, List& list, type_binary_size length);
Webss parseBinary(It& it, const ParamBinary& bhead, function<Webss()> func);
Webss parseBinaryElement(It& it, const ParamBinary::SizeHead& bhead);
Tuple parseBinaryFunction(It& it, const FunctionHeadBinary::Tuple& parameters);

type_binary_size checkBinarySize(type_int sizeInt);

void setDefaultValueBinary(Tuple& tuple, const FunctionHeadBinary::Tuple& defaultTuple, FunctionHeadBinary::Tuple::size_type index)
{
	tuple[index] = Webss(defaultTuple[index].sizeHead.defaultValue);
}

#define BINARY_DEFAULT_VALUE 1

//called only from parseFunctionHeadBinary (parserFunctions.cpp)
//dependency: parseValueEqual
void Parser::parseBinaryHead(It& it, FunctionHeadBinary& fhead)
{
	using bhead_t = ParamBinary::SizeHead;
	using blist_t = ParamBinary::SizeList;
	using Flag = bhead_t::Flag;
	auto sizeHead = parseBinarySizeHead(it);

	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	auto keyPair = parseKey(it);
	blist_t blist;
	switch (keyPair.second)
	{
	case KeyType::LIST:
		new (&blist) blist_t(parseBinarySizeList(++it));
		skipJunkToValid(it);
		if (*it != CHAR_EQUAL)
		{
			if (sizeHead.flag == Flag::SELF)
				throw runtime_error(webss_ERROR_EXPECTED_CHAR(CHAR_EQUAL));
			fhead.attach(move(keyPair.first), ParamBinary(move(sizeHead), move(blist)));
			return;
		}
		break;
	case KeyType::EQUAL:
		new (&blist) blist_t(blist_t::Type::ONE);
		break;
	case KeyType::KEYNAME:
		if (sizeHead.flag == Flag::SELF)
			throw runtime_error(webss_ERROR_EXPECTED_CHAR(CHAR_EQUAL));
		fhead.attach(move(keyPair.first), ParamBinary(move(sizeHead), blist_t(blist_t::Type::ONE)));
		return;
	case KeyType::KEYWORD:
		throw runtime_error(ERROR_KEYWORD_KEY);
	case KeyType::VARIABLE:
		throw runtime_error("can't have variable as key");
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}

	sizeHead.setDefaultValue(Webss(parseValueEqual(++it, ConType::FUNCTION_HEAD)));
	if (sizeHead.flag != Flag::SELF)
		sizeHead.flag = Flag::DEFAULT;
	fhead.attach(move(keyPair.first), (ParamBinary(move(sizeHead), move(blist))));
}

//called only from parseFunction (parserFunctions.cpp)
//no dependency outside of parseBinary.cpp
Tuple Parser::parseFunctionBodyBinary(It& it, const FunctionHeadBinary::Tuple& parameters)
{
	if (*skipJunkToValid(it) == CHAR_EQUAL)
		skipJunkToValid(++it);
	if (*it != OPEN_TUPLE)
		throw runtime_error("first element of a binary function must be a tuple");

	auto tuple = parseBinaryFunction(++it, parameters);
	if (it != CLOSE_TUPLE)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CLOSE_TUPLE));
	++it;
	return tuple;
}

//reads a number following UTF-7 encoding thing
type_binary_size readNumber(SmartIterator& it)
{
	type_binary_size num = 0;
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
void readBytes(SmartIterator& it, type_binary_size num, char* value)
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

	const auto& parameters = bhead.sizeHead.getFunctionHead().getParameters();
	return parseBinary(it, bhead, [&]() { return parseBinaryFunction(it, parameters); });
}

void parseBitList(It& it, List& list, type_binary_size length)
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
		type_int value = 0;
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
			throw domain_error(ERROR_UNDEFINED);
		}
	}

	auto length = GET_BINARY_LENGTH(bhead);
	string value;
	value.resize(length);
	readBytes(it, length, const_cast<char*>(value.data()));
	return Webss(move(value));
}
#undef GET_BINARY_LENGTH

Tuple parseBinaryFunction(It& it, const FunctionHeadBinary::Tuple& parameters)
{
	using Bhead = ParamBinary::SizeHead;
	Tuple tuple(parameters.getSharedKeys());
	for (Tuple::size_type i = 0; i < tuple.size(); ++i)
	{
		const auto& bhead = parameters[i];
		if (bhead.sizeHead.flag == Bhead::Flag::NONE)
			tuple[i] = parseBinary(it, bhead);
		else if (readByte(it) == BINARY_DEFAULT_VALUE)
			setDefaultValueBinary(tuple, parameters, i);
		else
			tuple[i] = bhead.sizeHead.flag == Bhead::Flag::DEFAULT ? parseBinary(it, bhead) : parseBinaryFunction(it, parameters);
	}
	return tuple;
}

//dependency: variables
ParamBinary::SizeHead Parser::parseBinarySizeHead(It& it)
{
	using bhead_t = ParamBinary::SizeHead;
	using Type = bhead_t::Type;
	using Flag = bhead_t::Flag;
	skipJunkToValid(it);
	Flag flag = *it == CHAR_SELF ? Flag::SELF : Flag::NONE;
	if (flag != Flag::NONE)
		skipJunkToValid(++it);

	bhead_t bhead;
	if (*it == CLOSE_TUPLE)
		new (&bhead) bhead_t(Type::EMPTY);
	else if (isNameStart(*it))
	{
		auto name = parseName(it);
		if (isKeyword(name))
		{
			Keyword keyword(name);
			if (!keyword.isType())
				throw runtime_error("invalid binary type: " + keyword.toString());
			if (keyword == Keyword::STRING)
				new (&bhead) bhead_t(Type::EMPTY);
			else 
				new (&bhead) bhead_t(keyword);
		}
		else if (vars.hasVariable(name))
		{
			const auto& content = vars[name].getContent();
			switch (vars[name].getContent().getType())
			{
			case WebssType::FUNCTION_HEAD_BINARY:
				new (&bhead) bhead_t(checkVariableFunctionHeadBinary(name));
				break;
			case WebssType::PRIMITIVE_INT:
				new (&bhead) bhead_t(checkVariableTypeBinarySize(name));
				break;
			default:
				throw runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
		}
		else
			throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(name));
	}
	else if (isNumberStart(*it))
		new (&bhead) bhead_t(checkBinarySize(parseNumber(it).getInt()));
	else if (*it == OPEN_FUNCTION)
	{
		auto headSwitch = parseFunctionHead(++it);
		if (!headSwitch.isBinary())
			throw runtime_error(ERROR_BINARY_SIZE_HEAD);
		new (&bhead) bhead_t(move(headSwitch.fheadBinary));
	}
	else
		throw runtime_error(ERROR_UNEXPECTED);

	skipJunkToValidCondition(it, [&]() { return *it == CLOSE_TUPLE; });
	bhead.flag = flag;
	++it;
	return bhead;
}

//dependency: variables
ParamBinary::SizeList Parser::parseBinarySizeList(It& it)
{
	using blist_t = ParamBinary::SizeList;
	using Type = blist_t::Type;
	if (checkEmptyContainer(it, ConType::LIST))
		return blist_t(Type::EMPTY);

	blist_t blist;
	if (isNameStart(*it))
	{
		auto name = parseName(it);
		if (vars.hasVariable(name))
		{
			if (!vars[name].getContent().isInt())
				throw runtime_error(ERROR_BINARY_SIZE_LIST);
			new (&blist) blist_t(checkVariableTypeBinarySize(name));
		}
		else if (isKeyword(name))
			new (&blist) blist_t(Keyword(name).getSize());
		else
			throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(name));
	}
	else if (isNumberStart(*it))
		new (&blist) blist_t(checkBinarySize(parseNumber(it).getInt()));
	else
		throw runtime_error(ERROR_UNEXPECTED);

	skipJunkToValidCondition(it, [&]() { return *it == CLOSE_LIST; });
	++it;
	return blist;
}

const BasicVariable<type_binary_size>& Parser::checkVariableTypeBinarySize(const string& name)
{
	if (!varsTypeBinarySize.hasVariable(name))
		try { varsTypeBinarySize.add(name, checkBinarySize(vars[name].getContent().getInt())); }
		catch (exception e) { throw runtime_error(e.what()); }
	return varsTypeBinarySize[name];
}

type_binary_size checkBinarySize(type_int sizeInt)
{
	if (sizeInt > numeric_limits<type_binary_size>::max())
		throw runtime_error("binary size is too big");
	else if (sizeInt < 0)
		throw runtime_error("binary size must be positive");
	return static_cast<type_binary_size>(sizeInt);
}

#undef BINARY_DEFAULT_VALUE