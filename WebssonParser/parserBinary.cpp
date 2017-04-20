//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include <limits>
#include <cstdint>
#include <cmath>

#include "errors.h"
#include "patternsContainers.h"
#include "WebssonUtils/constants.h"
#include "WebssonUtils/utilsWebss.h"
#include "binaryIterator.h"

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

const Entity& Parser::checkEntTypeBinarySize(const Entity& ent)
{
	try { checkBinarySize(ent.getContent().getIntSafe()); }
	catch (const exception& e) { throw runtime_error(e.what()); }
	return ent;
}

WebssBinarySize checkBinarySize(WebssInt sizeInt)
{
	if (sizeInt > numeric_limits<WebssBinarySize>::max())
		throw runtime_error("binary size is too big");
	else if (sizeInt < 0)
		throw runtime_error("binary size must be positive");
	return static_cast<WebssBinarySize>(sizeInt);
}

//entry point from parserTemplates
Tuple Parser::parseTemplateTupleBinary(const TemplateHeadBinary::Parameters& params)
{
	BinaryIterator itBin(it);
	auto tuple = parseBinaryTemplate(it, params);
	if (++it != CLOSE_TUPLE)
		throw runtime_error(webss_ERROR_EXPECTED_CHAR(CLOSE_TUPLE));
	++it;
	return tuple;
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
			tuple[i] = param.getSizeHead().isSelf() ? parseBinaryTemplate(it, params) : parseBinary(it, param);
	}
	return tuple;
}

void setDefaultValueBinary(Webss& value, const ParamBinary& param)
{
	value = Webss(param.getSizeHead().getDefaultPointer());
}

Webss parseBinary(BinaryIterator& it, const ParamBinary& param)
{
	if (!param.getSizeHead().isTemplateHead())
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
	if (bhead.isKeyword())
		return parseBinaryKeyword(it, bhead.getKeyword());
	
	if (bhead.isEmpty())
		return Webss(it.readString(it.readNumber()));

	auto length = bhead.size();
	if (length < 8)
	{
		string value;
		value += (char)it.readBits(length);
		return Webss(move(value));
	}

	return Webss(it.readString(std::ceil(length / 8)));
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