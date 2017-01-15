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

//DONE
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
	case CHAR_COLON:
		if (++it != CHAR_COLON || *skipJunkToValid(++it) != OPEN_TUPLE)
			throw runtime_error(ERROR_UNEXPECTED);
		return functionParseTupleText(++it, defaultTuple);
	case OPEN_TUPLE:
		return defaultTuple.containerText ? functionParseTupleText(++it, defaultTuple) : functionParseTuple(++it, defaultTuple);
	case OPEN_LIST:
		return defaultTuple.containerText ? FUNC_LIST(functionParseTupleText) : FUNC_LIST(functionParseTuple);
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
#undef FUNC_LIST
}

#define CON ConType::TUPLE

Webss Parser::parseFunctionContainer(It& it, const ParamStandard& defaultValue)
{
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

void setVoid(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index)
{
	if (tuple.at(index).t == WebssType::NONE)
		setDefaultValueSafe(tuple, defaultTuple, index);
}

void checkNoNone(Tuple& tuple)
{
	for (const Webss& webss : tuple)
		if (webss.t == WebssType::NONE)
			throw runtime_error(ERROR_NO_DEFAULT);
}

Tuple Parser::functionParseTuple(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	if (checkEmptyContainerVoid(it, CON))
		return makeDefaultTuple(defaultTuple);

	Tuple tuple(defaultTuple.getSharedKeys());
	Tuple::size_type index = 0;
	do
	{
		switch (*it)
		{
		case CLOSE_TUPLE:
			checkContainerEndVoid(it, [&]() { setVoid(tuple, defaultTuple, index++); });
			while (index < tuple.size())
				setVoid(tuple, defaultTuple, index++);
			checkNoNone(tuple);
			return tuple;
		case webss_CHAR_ANY_CONTAINER_CHAR_VALUE:
			tuple.at(index) = parseFunctionContainer(it, defaultTuple.at(index));
			break;
		case CHAR_POINTER: //named access
		{
			if (!isNameStart(*skipJunkToValid(it)))
				throw runtime_error(ERROR_UNEXPECTED);
			auto name = parseName(it);
			tuple.at(name) = parseFunctionContainer(it, defaultTuple.at(name));
			break;
		}
		case CHAR_SELF: //allow named acces like for pointer, perhaps self should be removed, but it does have a real use with binary functions...
		{				//what I need to do is allow binary to access a specific fhead within the binary data; nvm not possible
			if (!isNameStart(*skipJunkToValid(it)))
				tuple.at(index) = parseFunctionContainer(++it, FunctionHeadStandard(defaultTuple));
			else
			{
				auto name = parseName(it);
				tuple.at(name) = parseFunctionContainer(++it, FunctionHeadStandard(defaultTuple));
			}
			break;
		}
		default:
			if (checkSeparatorVoid(it, [&]() { setVoid(tuple, defaultTuple, index++); }))
				continue;

			parseOtherValue(it, CON,
				CaseKeyValue{ throw runtime_error(ERROR_UNEXPECTED); },
				CaseKeyOnly{ throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(key)); },
				CaseValueOnly
				{
					if (defaultTuple.at(index).hasFunctionHead())
						throw runtime_error(ERROR_UNEXPECTED);
					tuple.at(index) = move(value);
				},
				CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
			break;
		}
		++index;
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_EXPECTED);
}

Tuple Parser::functionParseTupleText(It& it, const FunctionHeadStandard::Tuple& defaultTuple)
{
	if (checkEmptyContainerVoid(it, CON))
		return makeDefaultTuple(defaultTuple);

	Tuple tuple(defaultTuple.getSharedKeys(), defaultTuple.containerText);
	Tuple::size_type index = 0;
	do
	{
		switch (*it)
		{
		case CLOSE_TUPLE:
			checkContainerEndVoid(it, [&]() { setDefaultValueSafe(tuple, defaultTuple, index++); });
			while (index < tuple.size())
				setDefaultValue(tuple, defaultTuple, index++);
			checkNoNone(tuple);
			return tuple;
		default:
			if (checkSeparatorVoid(it, [&]() { setDefaultValueSafe(tuple, defaultTuple, index++); }))
				continue;
			tuple.at(index++) = parseLineString(it, CON);
		}
		checkToNextElement(it, CON);
	} while (it);
	throw runtime_error(ERROR_EXPECTED);
}

#undef CON

List Parser::functionParseList(It& it, const FunctionHeadStandard::Tuple& defaultTuple, function<Tuple(It& it, const FunctionHeadStandard::Tuple& defaultTuple)> func)
{
	static const ConType CON = ConType::LIST;
	List list;
	if (checkEmptyContainer(it, CON))
		return list;
	do
		if (*it == OPEN_TUPLE)
			list.add(func(++it, defaultTuple));
		else if (*it == CHAR_COLON && ++it == CHAR_COLON && *skipJunkToValid(++it) == OPEN_TUPLE)
			list.add(functionParseTupleText(++it, defaultTuple));
		else
			throw runtime_error(ERROR_UNEXPECTED);
	while (checkNextElementContainer(it, CON)); //make it so separators are not required (no need to clean line)
	return list;
}