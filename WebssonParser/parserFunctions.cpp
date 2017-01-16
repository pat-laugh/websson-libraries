//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

Webss Parser::parseFunction(It& it)
{
	using Type = FunctionHeadSwitch::Type;
	auto headSwitch = parseFunctionHead(it);
	switch (headSwitch.t)
	{
	case Type::STANDARD:
	{
		auto head = move(headSwitch.fheadStandard);
		auto body = parseFunctionBodyStandard(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case Type::BINARY:
	{
		auto head = move(headSwitch.fheadBinary);
		auto body = parseFunctionBodyBinary(it, head.getParameters());
		return FunctionBinary(move(head), move(body));
	}
	default:
		throw logic_error("");
	}
}

Webss Parser::parseFunctionText(It& it)
{
	auto head = parseFunctionHeadText(it);
	auto body = parseFunctionBodyStandard(it, head.getParameters());
	return{ move(head), move(body) };
}

Webss Parser::parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
#define FUNC_LIST(parseTuple) functionParseList(++it, defaultTuple, [&](It& it, const FunctionHeadStandard::Tuple& defaultTuple) { return parseTuple(it, defaultTuple); })
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY:

	case OPEN_LIST:
		return defaultTuple.containerText ? FUNC_LIST(parseFunctionTupleText) : FUNC_LIST(parseFunctionTupleStandard);
	case OPEN_TUPLE:
		return defaultTuple.containerText ? parseFunctionTupleText(++it, defaultTuple) : parseFunctionTupleStandard(++it, defaultTuple);
	case CHAR_COLON:
		if (++it != CHAR_COLON || *skipJunkToValid(++it) != OPEN_TUPLE)
			throw runtime_error(ERROR_UNEXPECTED);
		return parseFunctionTupleText(++it, defaultTuple);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
#undef FUNC_LIST
}





Dictionary Parser::parseFunctionDictionaryStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::DICTIONARY;
	Dictionary dict;
	if (checkEmptyContainer(it, CON))
		return dict;
	do
	{
		if (!isNameStart(*it))
			throw runtime_error(ERROR_UNEXPECTED);
		auto name = parseNameType(it);
		if (name.type != NameType::NAME)
			throw runtime_error(ERROR_UNEXPECTED);
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::TUPLE:
			dict.addSafe(move(name.name), parseFunctionTupleStandard(++it, defaultTuple));
			break;
		case TypeContainer::LIST:
			dict.addSafe(move(name.name), parseFunctionListStandard(++it, defaultTuple));
			break;
		case TypeContainer::TEXT_TUPLE:
			dict.addSafe(move(name.name), parseFunctionTupleText(++it, defaultTuple));
			break;
		case TypeContainer::TEXT_LIST:
			dict.addSafe(move(name.name), parseFunctionListText(++it, defaultTuple));
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	} while (checkNextElementContainer(it, CON));
	return dict;
}

Dictionary Parser::parseFunctionDictionaryText(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::DICTIONARY;
	Dictionary dict;
	if (checkEmptyContainer(it, CON))
		return dict;
	do
	{
		if (!isNameStart(*it))
			throw runtime_error(ERROR_UNEXPECTED);
		auto name = parseNameType(it);
		if (name.type != NameType::NAME)
			throw runtime_error(ERROR_UNEXPECTED);
		if (*skipJunkToValid(it) == OPEN_TUPLE)
			dict.addSafe(move(name.name), parseFunctionTupleText(it, defaultTuple));
		else if (*it == OPEN_LIST)
			dict.addSafe(move(name.name), parseFunctionListText(it, defaultTuple));
		else if (*it == CHAR_COLON && ++it == CHAR_COLON)
		{
			if (*skipJunkToValid(it) == OPEN_TUPLE)
				dict.addSafe(move(name.name), parseFunctionTupleText(it, defaultTuple));
			else if (*it == OPEN_LIST)
				dict.addSafe(move(name.name), parseFunctionListText(it, defaultTuple));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		}
		else
			throw runtime_error(ERROR_UNEXPECTED);
	} while (checkNextElementContainer(it, CON));
	return dict;
}

List Parser::parseFunctionListStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::LIST;
	List list;
	if (checkEmptyContainer(it, CON))
		return list;
	do
		if (*it == OPEN_TUPLE)
			list.add(parseFunctionTupleStandard(++it, defaultTuple));
		else if (*it == CHAR_COLON && ++it == CHAR_COLON && *skipJunkToValid(++it) == OPEN_TUPLE)
			list.add(parseFunctionTupleText(++it, defaultTuple));
		else
			throw runtime_error(ERROR_UNEXPECTED);
		while (checkNextElementContainer(it, CON)); //make it so separators are not required (no need to clean line)
	return list;
}

List Parser::parseFunctionListText(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::LIST;
	List list;
	if (checkEmptyContainer(it, CON))
		return list;
	do
		if (*it == OPEN_TUPLE)
			list.add(parseFunctionTupleText(++it, defaultTuple));
		else if (*it == CHAR_COLON && ++it == CHAR_COLON && *skipJunkToValid(++it) == OPEN_TUPLE)
			list.add(parseFunctionTupleText(++it, defaultTuple));
		else
			throw runtime_error(ERROR_UNEXPECTED);
		while (checkNextElementContainer(it, CON)); //make it so separators are not required (no need to clean line)
	return list;
}










Webss Parser::parseFunctionContainer(It& it, const ParamStandard& defaultValue)
{
	static const ConType CON = ConType::TUPLE;
	using Type = ParamStandard::TypeFhead;
	switch (defaultValue.getTypeFhead())
	{
	case Type::STANDARD:
		return parseFunctionBodyStandard(it, defaultValue.getFunctionHeadStandard().getParameters());
	case Type::BINARY:
		return parseFunctionBodyBinary(it, defaultValue.getFunctionHeadBinary().getParameters());
	default:
		return parseValueEqual(it, CON);
	}
}

void checkDefaultValues(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).t == WebssType::NONE)
			setDefaultValue(tuple, defaultTuple, index);
}

Tuple Parser::parseFunctionTupleStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::TUPLE;
	Tuple tuple(defaultTuple.getSharedKeys());
	Tuple::size_type index = 0;
	if (!checkEmptyContainerVoid(it, CON, [&]() { ++index; }))
		do
		{
			switch (*it)
			{
			case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
				tuple.at(index) = parseFunctionContainer(it, defaultTuple.at(index));
				break;
			case CHAR_POINTER:
			{
				if (!isNameStart(*skipJunkToValid(++it)))
					throw runtime_error(ERROR_UNEXPECTED);
				auto nameType = parseNameType(it);
				if (nameType.type != NameType::NAME)
					throw runtime_error("expected param name");
				switch (*skipJunkToValid(it))
				{
				case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
					tuple.at(nameType.name) = parseFunctionContainer(it, defaultTuple.at(nameType.name));
					break;
				default:
					tuple.at(nameType.name) = parseCharValue(it, CON);
					break;
				}
				break;
			}
			case CHAR_SELF:
			{
				if (!isNameStart(*skipJunkToValid(it)))
				{
					switch (*it)
					{
					case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
						tuple.at(index) = parseFunctionContainer(it, FunctionHeadStandard(defaultTuple));
						break;
					default:
						throw runtime_error(ERROR_UNEXPECTED);
					}
				}
				else
				{
					auto nameType = parseNameType(it);
					if (nameType.type != NameType::NAME)
						throw runtime_error("expected param name");
					switch (*skipJunkToValid(it))
					{
					case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
						tuple.at(nameType.name) = parseFunctionContainer(it, FunctionHeadStandard(defaultTuple));
						break;
					default:
						throw runtime_error(ERROR_UNEXPECTED);
					}
				}
				break;
			}
			default:
				parseOtherValue(it, CON,
					ErrorKeyValue(ERROR_UNEXPECTED),
					ErrorKeyOnly(webss_ERROR_UNDEFINED_KEYNAME(key)),
					CaseValueOnly
				{
					if (defaultTuple.at(index).hasFunctionHead())
					throw runtime_error(ERROR_UNEXPECTED);
				tuple.at(index) = move(value);
				},
					ErrorAbstractEntity(ERROR_UNEXPECTED));
				break;
			}
			++index;
		} while (checkNextElementContainerVoid(it, CON, [&]() { ++index; }));
	checkDefaultValues(tuple, defaultTuple);
	return tuple;
}


Tuple Parser::parseFunctionTupleText(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	static const ConType CON = ConType::TUPLE;
	Tuple tuple(defaultTuple.getSharedKeys(), defaultTuple.containerText);
	Tuple::size_type index = 0;
	if (!checkEmptyContainerVoid(it, CON, [&]() { ++index; }))
		do
			tuple.at(index++) = parseLineString(it, CON);
	while (checkNextElementContainerVoid(it, CON, [&]() { ++index; }));
	checkDefaultValues(tuple, defaultTuple);
	return tuple;
}