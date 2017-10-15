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

using namespace std;
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
static ParamBin::SizeList parseBinSizeList(Parser& parser);

void Parser::parseBinHead(TheadBin& thead)
{
	using Bhead = ParamBin::SizeHead;
	using Blist = ParamBin::SizeList;

	Bhead bhead;
	Blist blist;
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
		blist = Blist(Blist::Type::ONE);
		goto goPastSeparatorCheck;
	case Tag::START_TEMPLATE_BIN_ARRAY:
		bhead = Bhead(Bhead::Type::EMPTY);
		blist = Blist(parseBinSizeList(*this));
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
		blist = Blist(parseBinSizeList(*this));
	else
		blist = Blist(Blist::Type::ONE);
	
goPastArrayCheck:
	tagit.sofertTag(Tag::TEMPLATE_BIN_SEPARATOR);
goPastSeparatorCheck:
	++tagit;
	parseExplicitKeyValue(
		CaseKeyValue
		{
			bhead.setDefaultValue(move(value));
			thead.attach(move(key), ParamBin(move(bhead), move(blist)));
		},
		CaseKeyOnly
		{
			if (bhead.isTheadSelf())
				throw runtime_error("binary param declared with self must have a default value");
			thead.attach(move(key), ParamBin(move(bhead), move(blist)));
		});
}

//entry point from parserTemplates
Tuple Parser::parseTemplateTupleBin(const TheadBin::Params& params)
{
	BinIterator itBin(getItSafe());
	auto tuple = parseBinTemplate(itBin, params);
	if (++getIt() != CHAR_END_TUPLE)
		throw runtime_error("binary tuple is not closed");
	tagit.incSafe();
	return tuple;
}

static ParamBin::SizeList parseBinSizeList(Parser& parser)
{
	using Blist = ParamBin::SizeList;
	Parser::ContainerSwitcher switcher(parser, ConType::TEMPLATE_BIN_ARRAY, false);
	if (parser.containerEmpty())
		return Blist(Blist::Type::EMPTY);

	Blist blist;
	try
	{
		if (*parser.tagit == Tag::NAME_START)
		{
			auto nameType = parseNameType(parser.tagit, parser.getEnts());
			if (nameType.type != NameType::ENTITY_CONCRETE)
				throw;
			blist = Blist(checkEntTypeBinSize(nameType.entity));
		}
		else if (*parser.tagit == Tag::DIGIT || *parser.tagit == Tag::PLUS || *parser.tagit == Tag::MINUS)
			blist = Blist(checkBinSize(parseNumber(parser).getInt()));
		else
			throw;

		parser.tagit.sofertTag(Tag::END_TEMPLATE_BIN_ARRAY);
		++parser.tagit;
	}
	catch (const exception&)
	{
		throw runtime_error("value in binary list must be void or a positive integer");
	}
	return blist;
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
	const auto& blist = param.getSizeList();
	if (blist.isOne())
		return func();

	List list;
	auto length = blist.isEmpty() ? it.readNumber() : blist.size();
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
	default:
		assert(false && "other keywords should've been parsed before"); throw domain_error("");
	}
}
