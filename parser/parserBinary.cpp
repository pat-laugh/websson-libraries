//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include <limits>
#include <cstdint>
#include <cmath>

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parserNumbers.hpp"
#include "patternsContainers.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "binaryIterator.hpp"

using namespace std;
using namespace webss;

WebssBinarySize checkBinarySize(WebssInt sizeInt);
WebssBinarySize checkBinarySizeBits(WebssInt sizeInt);
Tuple parseBinaryTemplate(BinaryIterator& it, const TemplateHeadBinary::Parameters& params);
Webss parseBinary(BinaryIterator& it, const ParamBinary& bhead);
Webss parseBinary(BinaryIterator& it, const ParamBinary& bhead, function<Webss()> func);
Webss parseBinaryElement(BinaryIterator& it, const ParamBinary::SizeHead& bhead);
Webss parseBinaryKeyword(BinaryIterator& it, Keyword keyword);
const Entity& checkEntTypeBinarySize(const Entity& ent);
const Entity& checkEntTypeBinarySizeBits(const Entity& ent);
ParamBinary::SizeList parseBinarySizeList(Parser& parser);

void Parser::parseBinaryHead(TemplateHeadBinary& thead)
{
	using Bhead = ParamBinary::SizeHead;
	using Blist = ParamBinary::SizeList;

	Bhead bhead;
	Blist blist;
	if (++tagit == Tag::END_TUPLE)
	{
		bhead = Bhead(Bhead::Type::EMPTY);
		blist = Blist(Blist::Type::ONE);
	}
	else if (*tagit == Tag::START_LIST)
	{
		bhead = Bhead(Bhead::Type::EMPTY);
		blist = Blist(parseBinarySizeList(*this));
	}
	else
	{
		switch (*tagit)
		{
		case Tag::NAME_START:
		{
			auto nameType = parseNameType(tagit, ents);
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
				throw runtime_error("undefined entity: " + nameType.name);
			}
			break;
		}
		case Tag::DIGIT: case Tag::PLUS: case Tag::MINUS:
			bhead = Bhead(checkBinarySize(parseNumber(*this).getInt()));
			break;
		case Tag::START_TEMPLATE:
		{
			auto headWebss = parseThead(true);
			switch (headWebss.getTypeRaw())
			{
			case TypeThead::BINARY:
				bhead = Bhead(move(headWebss.getTheadBinaryRaw()));
				break;
			case TypeThead::SELF:
				bhead = Bhead(TemplateHeadSelf());
				break;
			default:
				throw runtime_error(ERROR_BINARY_SIZE_HEAD);
			}
			break;
		}
		case Tag::EXPLICIT_NAME:
			if (++tagit == Tag::NAME_START)
			{
				auto nameType = parseNameType(tagit, ents);
				switch (nameType.type)
				{
				case NameType::KEYWORD: case NameType::ENTITY_ABSTRACT:
					throw runtime_error("invalid binary type: " + nameType.keyword.toString());
				case NameType::ENTITY_CONCRETE:
					bhead = Bhead::makeEntityBits(checkEntTypeBinarySizeBits(nameType.entity));
					break;
				default:
					throw runtime_error("undefined entity: " + nameType.name);
				}
			}
			else if (*tagit == Tag::DIGIT || *tagit == Tag::PLUS || *tagit == Tag::MINUS)
				bhead = Bhead::makeSizeBits(checkBinarySizeBits(parseNumber(*this).getInt()));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		default:
			break;
		}

		if (tagit.getSafe() == Tag::START_LIST)
			blist = Blist(parseBinarySizeList(*this));
		else
			blist = Blist(Blist::Type::ONE);
	}

	tagit.sofertTag(Tag::END_TUPLE);
	++tagit;
	parseExplicitKeyValue(
		CaseKeyValue
		{
			bhead.setDefaultValue(move(value));
			thead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		},
		CaseKeyOnly
		{
			if (bhead.isTemplateHeadSelf())
				throw runtime_error("binary param declared with self must have a default value");
			thead.attach(move(key), ParamBinary(move(bhead), move(blist)));
		});
}

ParamBinary::SizeList parseBinarySizeList(Parser& parser)
{
	using Blist = ParamBinary::SizeList;
	Parser::ContainerSwitcher switcher(parser, ConType::LIST, false);
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
			blist = Blist(checkEntTypeBinarySize(nameType.entity));
		}
		else if (*parser.tagit == Tag::DIGIT || *parser.tagit == Tag::PLUS || *parser.tagit == Tag::MINUS)
			blist = Blist(checkBinarySize(parseNumber(parser).getInt()));
		else
			throw;

		parser.tagit.sofertTag(Tag::END_LIST);
		++parser.tagit;
	}
	catch (const exception&)
	{
		throw runtime_error("value in binary list must be void or a positive integer");
	}
	return blist;
}

const Entity& checkEntTypeBinarySize(const Entity& ent)
{
	try { checkBinarySize(ent.getContent().getInt()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}

WebssBinarySize checkBinarySize(WebssInt sizeInt)
{
	if (static_cast<WebssBinarySize>(sizeInt) > numeric_limits<WebssBinarySize>::max())
		throw runtime_error("binary size is too big");
	else if (sizeInt < 0)
		throw runtime_error("binary size must be positive");
	return static_cast<WebssBinarySize>(sizeInt);
}

const Entity& checkEntTypeBinarySizeBits(const Entity& ent)
{
	try { checkBinarySizeBits(ent.getContent().getInt()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}

WebssBinarySize checkBinarySizeBits(WebssInt sizeInt)
{
	if (sizeInt < 1 || sizeInt > 8)
		throw runtime_error("the number of bits must be between 1 and 8");
	return static_cast<WebssBinarySize>(sizeInt);
}

//entry point from parserTemplates
Tuple Parser::parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params)
{
	BinaryIterator itBin(getItSafe());
	auto tuple = parseBinaryTemplate(itBin, params);
	if (++getIt() != CLOSE_TUPLE)
		throw runtime_error("binary tuple is not closed");
	tagit.incSafe();
	return tuple;
}

inline void setDefaultValueBinary(Webss& value, const ParamBinary& param)
{
	value = Webss(param.getSizeHead().getDefaultPointer());
}

Tuple parseBinaryTemplate(BinaryIterator& it, const TemplateHeadBinary::Parameters& params)
{
	Tuple tuple(params.getSharedKeys());
	for (decltype(tuple.size()) i = 0; i < tuple.size(); ++i)
	{
		const auto& param = params[i];
		if (!param.getSizeHead().hasDefaultValue())
			tuple[i] = parseBinary(it, param);
		else if (it.readBit() == CHAR_BINARY_DEFAULT_TRUE)
			setDefaultValueBinary(tuple[i], params[i]);
		else
			tuple[i] = param.getSizeHead().isTemplateHeadSelf() ? parseBinaryTemplate(it, params) : parseBinary(it, param);
	}
	return tuple;
}

Webss parseBinary(BinaryIterator& it, const ParamBinary& param)
{
	if (!param.getSizeHead().isTemplateHeadBinary())
		return parseBinary(it, param, [&]() { return parseBinaryElement(it, param.getSizeHead()); });

	const auto& params = param.getSizeHead().getTemplateHead().getParameters();
	return parseBinary(it, param, [&]() { return parseBinaryTemplate(it, params); });
}

Webss parseBinary(BinaryIterator& it, const ParamBinary& param, function<Webss()> func)
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

Webss parseBinaryElement(BinaryIterator& it, const ParamBinary::SizeHead& bhead)
{
	using Type = ParamBinary::SizeHead::Type;

	switch (bhead.getType())
	{
	case Type::KEYWORD:
		return parseBinaryKeyword(it, bhead.getKeyword());
	case Type::EMPTY: case Type::EMPTY_ENTITY_NUMBER:
		return Webss(it.readString(it.readNumber()));
	case Type::BITS: case Type::ENTITY_BITS:
		return Webss(string(1, (char)it.readBits(bhead.size())));
	case Type::NUMBER: case Type::ENTITY_NUMBER:
		return Webss(it.readString(bhead.size()));
#ifdef assert
	default:
		assert(false);
		throw domain_error("");
#endif
	}
}

Webss parseBinaryKeyword(BinaryIterator& it, Keyword keyword)
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
