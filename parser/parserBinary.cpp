//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include <limits>
#include <cstdint>
#include <cmath>

#include "binaryIterator.hpp"
#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parserNumbers.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "various/StringBuilder.hpp"

using namespace std;
using namespace various;
using namespace webss;

static WebssBinSize checkBinSize(WebssInt sizeInt);
static WebssBinSize checkBinSizeBits(WebssInt sizeInt);
static Tuple parseBinTemplate(BinIterator& it, const TheadBin::Params& params);
static Webss parseBin(BinIterator& it, const ParamBin& bhead);
static Webss parseBin(BinIterator& it, const ParamBin& bhead, function<Webss()> func);
static Webss parseBinElement(BinIterator& it, const ParamBin::SizeHead& bhead);
static Webss parseBinKeyword(BinIterator& it, Keyword keyword);
static const Entity& checkEntTypeBinSize(const Entity& ent);
static const Entity& checkEntTypeBinSizeBits(const Entity& ent);
static ParamBin::SizeArray parseBinSizeArray(Parser& parser);
static SmartIterator decodeBase64(SmartIterator& it);
static void decodeBase64(SmartIterator& it, char bytes[4]);

void Parser::parseBinHead(TheadBin& thead)
{
	using Bhead = ParamBin::SizeHead;
	using Barray = ParamBin::SizeArray;

	Bhead bhead;
	Barray barray;
	switch (*tagit)
	{
	case Tag::EXPAND:
	{
		const auto& content = parseExpandEntity(tagit, ents).getContent();
		if (!content.isThead() || !content.getThead().isTheadBin())
			throw std::runtime_error("expand entity within binary template head must be a binary template head");
		thead.attach(content.getThead().getTheadBin());
		return;
	}
	case Tag::TEMPLATE_BIN_SEPARATOR:
		bhead = Bhead(Bhead::Type::EMPTY);
		barray = Barray(Barray::Type::ONE);
		goto goPastSeparatorCheck;
	case Tag::START_TEMPLATE_BIN_ARRAY:
		bhead = Bhead(Bhead::Type::EMPTY);
		barray = Barray(parseBinSizeArray(*this));
		goto goPastArrayCheck;
	case Tag::NAME_START:
	{
		auto nameType = parseNameType(tagit, ents);
		switch (nameType.type)
		{
		case NameType::KEYWORD:
			throw runtime_error("invalid binary type: " + nameType.keyword.toString());
		case NameType::ENTITY_ABSTRACT:
			if (!nameType.entity.getContent().isThead() || !nameType.entity.getContent().getThead().isTheadBin())
				throw runtime_error(ERROR_UNEXPECTED);
			bhead = Bhead::makeEntityThead(nameType.entity);
			break;
		case NameType::ENTITY_CONCRETE:
			bhead = Bhead::makeEntityNumber(checkEntTypeBinSize(nameType.entity));
			break;
		default:
			throw runtime_error("undefined entity: " + nameType.name);
		}
		break;
	}
	case Tag::DIGIT: case Tag::PLUS: case Tag::MINUS:
		bhead = Bhead(checkBinSize(parseNumber(*this).getInt()));
		break;
	case Tag::START_TEMPLATE:
	{
		auto headWebss = parseThead(true);
		switch (headWebss.getTypeRaw())
		{
		case TypeThead::BIN:
			bhead = Bhead(move(headWebss.getTheadBinRaw()));
			break;
		case TypeThead::SELF:
			bhead = Bhead(TheadSelf());
			break;
		default:
			throw runtime_error(ERROR_BIN_SIZE_HEAD);
		}
		break;
	}
	case Tag::EXPLICIT_NAME:
	{
		auto& it = ++tagit.getItSafe();
		if (!it)
			throw runtime_error(ERROR_EXPECTED);
		if (isNameStart(*it))
		{
			auto nameType = parseNameType(tagit, ents);
			switch (nameType.type)
			{
			case NameType::KEYWORD: case NameType::ENTITY_ABSTRACT:
				throw runtime_error("invalid binary type: " + nameType.keyword.toString());
			case NameType::ENTITY_CONCRETE:
				bhead = Bhead::makeEntityBits(checkEntTypeBinSizeBits(nameType.entity));
				break;
			default:
				throw runtime_error("undefined entity: " + nameType.name);
			}
		}
		else if (isNumberStart(*it))
			bhead = Bhead::makeSizeBits(checkBinSizeBits(parseNumber(*this).getInt()));
		else
			throw runtime_error(ERROR_UNEXPECTED);
		break;
	}
	default:
		throw std::runtime_error("all values in a binary template must be binary");
	}

	if (tagit.getSafe() == Tag::START_TEMPLATE_BIN_ARRAY)
		barray = Barray(parseBinSizeArray(*this));
	else
		barray = Barray(Barray::Type::ONE);
	
goPastArrayCheck:
	tagit.sofertTag(Tag::TEMPLATE_BIN_SEPARATOR);
goPastSeparatorCheck:
	++tagit;
	parseExplicitKeyValue(
		CaseKeyValue
		{
			bhead.setDefaultValue(move(value));
			thead.attach(move(key), ParamBin(move(bhead), move(barray)));
		},
		CaseKeyOnly
		{
			if (bhead.isTheadSelf())
				throw runtime_error("binary param declared with self must have a default value");
			thead.attach(move(key), ParamBin(move(bhead), move(barray)));
		});
}

//entry point from parserTemplates
Tuple Parser::parseTemplateTupleBin(const TheadBin::Params& params, bool isEncoded)
{
	Tuple tuple;
	if (isEncoded)
	{
		auto itDecodedBinary = decodeBase64(getIt()); //this advances it until ')' is met and returns iterator containing decoded bytes
		BinIterator itBin(itDecodedBinary);
		try { tuple = parseBinTemplate(itBin, params); }
		catch (const runtime_error& e) { throw runtime_error(string("while parsing decoded binary, ") + e.what()); }
	}
	else
	{
		BinIterator itBin(getIt());
		try { tuple = parseBinTemplate(itBin, params); }
		catch (const runtime_error& e) { throw runtime_error(string("while parsing binary, ") + e.what()); }
		do
			if (!++getIt())
				throw runtime_error("binary tuple is not closed");
		while (*getIt() != CHAR_END_TUPLE);
	}
	++tagit;
	return tuple;
}

static ParamBin::SizeArray parseBinSizeArray(Parser& parser)
{
	using Barray = ParamBin::SizeArray;
	Parser::ContainerSwitcher switcher(parser, ConType::TEMPLATE_BIN_ARRAY, false);
	if (parser.containerEmpty())
		return Barray(Barray::Type::EMPTY);

	Barray barray;
	try
	{
		if (*parser.tagit == Tag::NAME_START)
		{
			auto nameType = parseNameType(parser.tagit, parser.getEntityManager());
			if (nameType.type != NameType::ENTITY_CONCRETE)
				throw;
			barray = Barray(checkEntTypeBinSize(nameType.entity));
		}
		else if (*parser.tagit == Tag::DIGIT || *parser.tagit == Tag::PLUS || *parser.tagit == Tag::MINUS)
			barray = Barray(checkBinSize(parseNumber(parser).getInt()));
		else
			throw;

		parser.tagit.sofertTag(Tag::END_TEMPLATE_BIN_ARRAY);
		++parser.tagit;
	}
	catch (const exception&)
	{
		throw runtime_error("value in binary array must be void or a positive integer");
	}
	return barray;
}

static const Entity& checkEntTypeBinSize(const Entity& ent)
{
	try { checkBinSize(ent.getContent().getInt()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}

static WebssBinSize checkBinSize(WebssInt sizeInt)
{
	if (static_cast<WebssBinSize>(sizeInt) > numeric_limits<WebssBinSize>::max())
		throw runtime_error("binary size is too big");
	else if (sizeInt < 0)
		throw runtime_error("binary size must be positive");
	return static_cast<WebssBinSize>(sizeInt);
}

static const Entity& checkEntTypeBinSizeBits(const Entity& ent)
{
	try { checkBinSizeBits(ent.getContent().getInt()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}

static WebssBinSize checkBinSizeBits(WebssInt sizeInt)
{
	if (sizeInt < 1 || sizeInt > 8)
		throw runtime_error("the number of bits must be between 1 and 8");
	return static_cast<WebssBinSize>(sizeInt);
}

static inline void setDefaultValueBin(Webss& value, const ParamBin& param)
{
	value = Webss(param.getSizeHead().getDefaultPointer());
}

static Tuple parseBinTemplate(BinIterator& it, const TheadBin::Params& params)
{
	Tuple tuple(params.getSharedKeys());
	for (decltype(tuple.size()) i = 0; i < tuple.size(); ++i)
	{
		const auto& param = params[i];
		if (!param.getSizeHead().hasDefaultValue())
			tuple[i] = parseBin(it, param);
		else if (it.readBit() == CHAR_BIN_DEFAULT_TRUE)
			setDefaultValueBin(tuple[i], params[i]);
		else
			tuple[i] = param.getSizeHead().isTheadSelf() ? parseBinTemplate(it, params) : parseBin(it, param);
	}
	return tuple;
}

static Webss parseBin(BinIterator& it, const ParamBin& param)
{
	if (!param.getSizeHead().isTheadBin())
		return parseBin(it, param, [&]() { return parseBinElement(it, param.getSizeHead()); });

	const auto& params = param.getSizeHead().getThead().getParams();
	return parseBin(it, param, [&]() { return parseBinTemplate(it, params); });
}

static Webss parseBin(BinIterator& it, const ParamBin& param, function<Webss()> func)
{
	const auto& barray = param.getSizeArray();
	if (barray.isOne())
		return func();

	List list;
	auto length = barray.isEmpty() ? it.readNumber() : barray.size();
	while (length-- > 0)
		list.add(func());

	return list;
}

static Webss parseBinElement(BinIterator& it, const ParamBin::SizeHead& bhead)
{
	using Type = ParamBin::SizeHead::Type;

	switch (bhead.getType())
	{
	case Type::KEYWORD:
		return parseBinKeyword(it, bhead.getKeyword());
	case Type::EMPTY: case Type::EMPTY_ENTITY_NUMBER:
		return Webss(it.readString(it.readNumber()));
	case Type::BITS: case Type::ENTITY_BITS:
		return Webss(string(1, (char)it.readBits(bhead.size())));
	case Type::NUMBER: case Type::ENTITY_NUMBER:
		return Webss(it.readString(bhead.size()));
	default:
		assert(false); throw domain_error("");
	}
}

static Webss parseBinKeyword(BinIterator& it, Keyword keyword)
{
	union
	{
		int16_t tInt16;
		int32_t tInt32;
		int64_t tInt64;
		float tFloat;
		double tDouble;
	};

	switch (keyword)
	{
	case Keyword::BOOL:
		return Webss(it.readBit() != 0);
	case Keyword::INT8:
		return Webss((WebssInt)it.readByte());
	case Keyword::INT16:
		it.readBytes(2, reinterpret_cast<char*>(&tInt16));
		return Webss(tInt16);
	case Keyword::INT32:
		it.readBytes(4, reinterpret_cast<char*>(&tInt32));
		return Webss(tInt32);
	case Keyword::INT64:
		it.readBytes(8, reinterpret_cast<char*>(&tInt64));
		return Webss(tInt64);
	case Keyword::FLOAT:
		it.readBytes(4, reinterpret_cast<char*>(&tFloat));
		return Webss(tFloat);
	case Keyword::DOUBLE:
		it.readBytes(8, reinterpret_cast<char*>(&tDouble));
		return Webss(tDouble);
	case Keyword::VARINT:
		return Webss(it.readNumber());
	default:
		assert(false && "other keywords should've been parsed before"); throw domain_error("");
	}
}

#define BASE_64_ALL_0 '+'
#define BASE_64_ALL_1 '-'
#define BASE_64_BYTES_0 "\x00\x00\x00"
#define BASE_64_BYTES_1 "\xff\xff\xff"
#define BASE_64_NUMBER '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9'
#define BASE_64_UPPER 'Q': case 'W': case 'F': case 'R': case 'K': case 'Y': case 'J': case 'I': case 'O': case 'P': \
	case 'A': case 'S': case 'D': case 'T': case 'G': case 'H': case 'E': case 'U': case 'L': \
	case 'Z': case 'X': case 'C': case 'V': case 'B': case 'N': case 'M'
#define BASE_64_LOWER 'q': case 'w': case 'f': case 'r': case 'k': case 'y': case 'j': case 'i': case 'o': case 'p': \
	case 'a': case 's': case 'd': case 't': case 'g': case 'h': case 'e': case 'u': case 'l': \
	case 'z': case 'x': case 'c': case 'v': case 'b': case 'n': case 'm'
#define BASE_64_CHAR_1 '_'
#define BASE_64_CHAR_2 '='
#define BASE_64_IGNORED ' ': case '\x7f': \
	case '\x00': case '\x01': case '\x02': case '\x03': case '\x04': case '\x05': case '\x06': case '\x07': case '\x08': case '\x09': case '\x0a': case '\x0b': case '\x0c': case '\x0d': case '\x0e': case '\x0f': \
	case '\x10': case '\x11': case '\x12': case '\x13': case '\x14': case '\x15': case '\x16': case '\x17': case '\x18': case '\x19': case '\x1a': case '\x1b': case '\x1c': case '\x1d': case '\x1e': case '\x1f'

static SmartIterator decodeBase64(SmartIterator& it)
{
	StringBuilder sb;
	union
	{
		char bytes[4];
		int32_t bytesInt;
	};
decodeStart:
	if (!it)
		throw runtime_error(ERROR_EXPECTED);
	switch (*it)
	{
	case CHAR_END_TUPLE:
		return sb.str();
	case BASE_64_ALL_0:
		sb += BASE_64_BYTES_0;
		goto decodeEnd;
	case BASE_64_ALL_1:
		sb += BASE_64_BYTES_1;
		goto decodeEnd;
	case BASE_64_IGNORED:
		goto decodeEnd;
	default:
		break;
	}
	decodeBase64(it, bytes);
	//the 4 bytes: 00111111|00222222|00333333|00112233
	//will become: 11111111|22222222|33333333|00000000
	bytesInt |= (bytesInt & 0x30) << 26;
	bytesInt |= (bytesInt & 0x0c) << 20;
	bytesInt |= (bytesInt & 0x03) << 14;
	bytes[3] = 0;
	sb += bytes;
decodeEnd:
	++it;
	goto decodeStart;
}

static void decodeBase64(SmartIterator& it, char bytes[4])
{
	int i = 0;
startSwitch:
	switch (*it)
	{
	case BASE_64_IGNORED:
		goto restart;
	case BASE_64_NUMBER:
		bytes[i] = *it - '0';
		break;
	case BASE_64_UPPER:
		bytes[i] = *it - 'A' + 10;
		break;
	case BASE_64_LOWER:
		bytes[i] = *it - 'a' + 36;
		break;
	case BASE_64_CHAR_1:
		bytes[i] = 62;
		break;
	case BASE_64_CHAR_2:
		bytes[i] = 63;
		break;
	default:
		throw runtime_error("forbidden character in encoded binary");
	}
	if (++i == 4)
		return;
restart:
	if (!++it)
		throw runtime_error(ERROR_EXPECTED);
	goto startSwitch;
}