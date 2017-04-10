//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <limits>

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"

using namespace std;
using namespace webss;

const char ERROR_BINARY_SIZE_HEAD[] = "size of binary head must be a positive integer, binary template head or equivalent entity";
const char ERROR_BINARY_SIZE_LIST[] = "size of binary list must be a positive integer or equivalent entity";

Webss parseBinary(SmartIterator& it, const ParamBinary& bhead);
void parseBitList(SmartIterator& it, List& list, WebssBinarySize length);
Webss parseBinary(SmartIterator& it, const ParamBinary& bhead, function<Webss()> func);
Webss parseBinaryElement(SmartIterator& it, const ParamBinary::SizeHead& bhead);
Tuple parseBinaryTemplate(SmartIterator& it, const TemplateHeadBinary::Parameters& params);

WebssBinarySize checkBinarySize(WebssInt sizeInt);

void Parser::parseBinaryHead(TemplateHeadBinary& thead)
{
	++it;
	using Bhead = ParamBinary::SizeHead;
	using Blist = ParamBinary::SizeList;

	Bhead bhead;
	Blist blist;
	if (*skipJunkToValid(it) == CLOSE_TUPLE)
	{
		bhead = Bhead(Bhead::Type::EMPTY);
		blist = Blist(Blist::Type::ONE);
	}
	else if (*it == OPEN_LIST)
	{
		bhead = Bhead(Bhead::Type::EMPTY);
		blist = Blist(parseBinarySizeList());
	}
	else
	{
		if (isNameStart(*it))
		{
			auto nameType = parseNameType();
			switch (nameType.type)
			{
			case NameType::KEYWORD:
				throw runtime_error("invalid binary type: " + nameType.keyword.toString());
			case NameType::ENTITY_ABSTRACT:
				if (!nameType.entity.getContent().isTemplateHeadBinary())
					throw runtime_error(ERROR_UNEXPECTED);
				bhead = Bhead::makeEntityThead(nameType.entity);
				break;
			case NameType::ENTITY_CONCRETE:
				bhead = Bhead::makeEntityNumber(checkEntTypeBinarySize(nameType.entity));
				break;
			default:
				throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(nameType.name));
			}
		}
		else if (isNumberStart(*it))
			bhead = Bhead(checkBinarySize(parseNumber().getIntSafe()));
		else if (*it == OPEN_TEMPLATE)
		{
			auto headWebss = parseTemplateHead();
			switch (headWebss.getType())
			{
			case WebssType::TEMPLATE_HEAD_BINARY:
				bhead = Bhead(move(headWebss.getTemplateHeadBinary()));
				break;
			case WebssType::TEMPLATE_HEAD_SELF:
				bhead = Bhead(TemplateHeadSelf());
				break;
			default:
				throw runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
		}

		if (*skipJunkToValid(it) == OPEN_LIST)
			blist = Blist(parseBinarySizeList());
		else
			blist = Blist(Blist::Type::ONE);
	}

	skipJunkToTag(it, Tag::END_TUPLE);
	skipJunkToValid(++it);
	parseOtherValue(
		CaseKeyValue
		{
			bhead.setDefaultValue(move(value));
			thead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		},
		CaseKeyOnly
		{
			if (bhead.isSelf())
				throw runtime_error("binary param declared with self must have a default value");
			thead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		},
		ErrorValueOnly(ERROR_ANONYMOUS_KEY),
		ErrorAbstractEntity(ERROR_ANONYMOUS_KEY));
}

//entry point from parserTemplates
Tuple Parser::parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params)
{
	++it;
	auto tuple = parseBinaryTemplate(it, params);
	if (it != CLOSE_TUPLE)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CLOSE_TUPLE));
	++it;
	return tuple;
}

//reads a number following UTF-7 encoding thing
WebssBinarySize readNumber(SmartIterator& it)
{
	const int fullShift = 7, maxFullShifts = sizeof(WebssBinarySize) / fullShift;
	WebssBinarySize num = 0;
	for (int numShifts = 0; ; ++it)
	{
		if (!it)
			throw runtime_error(ERROR_EXPECTED);
		num = (num << fullShift) | (0x7F & *it);
		if ((*it & 0x80) == 0)
			break;
		if (++numShifts == maxFullShifts)
		{
			const int partShift = sizeof(WebssBinarySize) % fullShift;
			const int partMask = 0xFF >> (8 - partShift);
			const int partEndMask = 0xFF ^ partMask;
			if (!++it)
				throw runtime_error(ERROR_EXPECTED);
			if ((*it & partEndMask) != 0)
				throw runtime_error("binary length is too great");
			num = (num << partShift) | (partMask & *it);
			break;
		}
	}
	++it;
	return checkBinarySize(num);
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

Webss parseBinary(SmartIterator& it, const ParamBinary& bhead)
{
	if (!bhead.getSizeHead().isTemplateHead())
		return parseBinary(it, bhead, [&]() { return parseBinaryElement(it, bhead.getSizeHead()); });

	const auto& params = bhead.getSizeHead().getTemplateHead().getParameters();
	return parseBinary(it, bhead, [&]() { return parseBinaryTemplate(it, params); });
}

void parseBitList(SmartIterator& it, List& list, WebssBinarySize length)
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

Webss parseBinaryKeyword(SmartIterator& it, Keyword keyword)
{
	union
	{
		WebssInt tInt;
		float tFloat;
		double tDouble;
	};

	switch (keyword)
	{
	case Keyword::BOOL:
		return Webss(readByte(it) != 0);
	case Keyword::INT8:
		return Webss((WebssInt)readByte(it));
	case Keyword::INT16: case Keyword::INT32: case Keyword::INT64:
		tInt = 0;
		readBytes(it, keyword.getSize() / 8, reinterpret_cast<char*>(&tInt));
		return Webss(tInt);
	case Keyword::FLOAT:
		readBytes(it, 4, reinterpret_cast<char*>(&tFloat));
		return Webss(tFloat);
	case Keyword::DOUBLE:
		readBytes(it, 8, reinterpret_cast<char*>(&tDouble));
		return Webss(tDouble);
	default:
		assert(false && "other keywords should've been parsed before"); throw domain_error("");
	}
}

#define getBinaryLength(x) x.isEmpty() ? readNumber(it) : x.size()

Webss parseBinary(SmartIterator& it, const ParamBinary& param, function<Webss()> func)
{
	if (param.getSizeList().isOne())
		return func();

	List list;
	auto length = getBinaryLength(param.getSizeList());
	if (param.getSizeHead().isBool())
		parseBitList(it, list, length);
	else
		while (length-- > 0)
			list.add(func());

	return list;
}

Webss parseBinaryElement(SmartIterator& it, const ParamBinary::SizeHead& bhead)
{
	if (bhead.isKeyword())
		return parseBinaryKeyword(it, bhead.getKeyword());

	auto length = getBinaryLength(bhead);
	string value(length, 0);
	readBytes(it, length, const_cast<char*>(value.data()));
	return Webss(move(value));
}

#undef getBinaryLength

void setDefaultValueBinary(Webss& value, const ParamBinary& param)
{
	value = Webss(param.getSizeHead().getDefaultPointer());
}

Tuple parseBinaryTemplate(SmartIterator& it, const TemplateHeadBinary::Parameters& params)
{
	using Bhead = ParamBinary::SizeHead;
	Tuple tuple(params.getSharedKeys());
	for (Tuple::size_type i = 0; i < tuple.size(); ++i)
	{
		const auto& bhead = params[i];
		if (!bhead.getSizeHead().hasDefaultValue())
			tuple[i] = parseBinary(it, bhead);
		else if ((unsigned char)readByte(it) >= CHAR_BINARY_DEFAULT_TRUE)
			setDefaultValueBinary(tuple[i], params[i]);
		else
			tuple[i] = bhead.getSizeHead().isSelf() ?  parseBinaryTemplate(it, params) : parseBinary(it, bhead);
	}
	return tuple;
}

ParamBinary::SizeList Parser::parseBinarySizeList()
{
	using Blist = ParamBinary::SizeList;
	ContainerSwitcher switcher(*this, ConType::LIST, false);
	if (containerEmpty())
		return Blist(Blist::Type::EMPTY);

	Blist blist;
	try
	{
		if (isNameStart(*it))
		{
			auto nameType = parseNameType();
			if (nameType.type != NameType::ENTITY_CONCRETE)
				throw;
			blist = Blist(checkEntTypeBinarySize(nameType.entity));
		}
		else if (isNumberStart(*it))
			blist = Blist(checkBinarySize(parseNumber().getIntSafe()));
		else
			throw;
	}
	catch (const exception&)
	{
		throw runtime_error("value in binary list must be void or a positive integer");
	}
	++skipJunkToTag(it, Tag::END_LIST);
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

const Entity& Parser::checkEntTypeBinarySize(const Entity& ent)
{
	try { checkBinarySize(ent.getContent().getIntSafe()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}