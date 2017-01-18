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
	auto body = parseFunctionBodyText(it, head.getParameters());
	return{ move(head), move(body) };
}

Webss Parser::parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	switch (skipJunkToContainer(it))
	{
	case TypeContainer::DICTIONARY:
		return parseFunctionDictionaryStandard(++it, defaultTuple);
	case TypeContainer::LIST:
		return parseFunctionListStandard(++it, defaultTuple);
	case TypeContainer::TUPLE:
		return parseFunctionTupleStandard(++it, defaultTuple);
	case TypeContainer::TEXT_DICTIONARY:
		return parseFunctionDictionaryText(++it, defaultTuple);
	case TypeContainer::TEXT_LIST:
		return parseFunctionListText(++it, defaultTuple);
	case TypeContainer::TEXT_TUPLE:
		return parseFunctionTupleText(++it, defaultTuple);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

Webss Parser::parseFunctionBodyText(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	switch (skipJunkToContainer(it))
	{
	case TypeContainer::DICTIONARY: case TypeContainer::TEXT_DICTIONARY:
		return parseFunctionDictionaryText(++it, defaultTuple);
	case TypeContainer::LIST: case TypeContainer::TEXT_LIST:
		return parseFunctionListText(++it, defaultTuple);
	case TypeContainer::TUPLE: case TypeContainer::TEXT_TUPLE:
		return parseFunctionTupleText(++it, defaultTuple);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
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
		auto name = parseNameSafe(it);
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::LIST:
			dict.addSafe(move(name), parseFunctionListStandard(++it, defaultTuple));
			break;
		case TypeContainer::TUPLE:
			dict.addSafe(move(name), parseFunctionTupleStandard(++it, defaultTuple));
			break;
		case TypeContainer::TEXT_LIST:
			dict.addSafe(move(name), parseFunctionListText(++it, defaultTuple));
			break;
		case TypeContainer::TEXT_TUPLE:
			dict.addSafe(move(name), parseFunctionTupleText(++it, defaultTuple));
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
		auto name = parseNameSafe(it);
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::LIST: case TypeContainer::TEXT_LIST:
			dict.addSafe(move(name), parseFunctionListText(++it, defaultTuple));
			break;
		case TypeContainer::TUPLE: case TypeContainer::TEXT_TUPLE:
			dict.addSafe(move(name), parseFunctionTupleText(++it, defaultTuple));
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
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
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::TUPLE:
			list.add(parseFunctionTupleStandard(++it, defaultTuple));
			break;
		case TypeContainer::TEXT_TUPLE:
			list.add(parseFunctionTupleText(++it, defaultTuple));
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
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
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::TUPLE: case TypeContainer::TEXT_TUPLE:
			list.add(parseFunctionTupleText(++it, defaultTuple));
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
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
	{
		const auto& params = defaultValue.getFunctionHeadStandard().getParameters();
		return params.containerText ? parseFunctionBodyText(it, params) : parseFunctionBodyStandard(it, params);
	}
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
				auto name = parseNameSafe(it);
				switch (*skipJunkToValid(it))
				{
				case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
					tuple.at(name) = parseFunctionContainer(it, defaultTuple.at(name));
					break;
				default:
					tuple.at(name) = parseCharValue(it, CON);
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
					auto name = parseNameSafe(it);
					switch (*skipJunkToValid(it))
					{
					case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
						tuple.at(name) = parseFunctionContainer(it, FunctionHeadStandard(defaultTuple));
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