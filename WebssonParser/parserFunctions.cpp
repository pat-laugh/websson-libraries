//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

void checkDefaultValues(Tuple& tuple, const FunctionHeadStandard::Tuple& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).t == WebssType::NONE)
			setDefaultValue(tuple, params, index);
}

class ParserFunctions : public Parser
{
public:
	Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& params)
	{
		return parseFunctionBody<FunctionHeadStandard::Tuple>(it, params, [&](It& it, const FunctionHeadStandard::Tuple& params) { return parseFunctionTupleStandard(it, params); }, [&](It& it, const FunctionHeadStandard::Tuple& params) { return parseFunctionTupleText(it, params); });
	}

	Webss parseFunctionBodyText(It& it, const FunctionHeadStandard::Tuple& params)
	{
		return parseFunctionBody<FunctionHeadStandard::Tuple>(it, params, [&](It& it, const FunctionHeadStandard::Tuple& params) { return parseFunctionTupleText(it, params); }, [&](It& it, const FunctionHeadStandard::Tuple& params) { return parseFunctionTupleText(it, params); });
	}

	template <class Parameters>
	Webss parseFunctionBody(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleRegular, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
	{
		switch (skipJunkToContainer(it))
		{
		case TypeContainer::DICTIONARY:
			return parseFunctionDictionary<Parameters>(++it, params, move(funcTupleRegular), move(funcTupleText));
		case TypeContainer::LIST:
			return parseFunctionListRegular<Parameters>(++it, params, move(funcTupleRegular), move(funcTupleText));
		case TypeContainer::TUPLE:
			return funcTupleRegular(++it, params);
		case TypeContainer::TEXT_LIST:
			return parseFunctionListText<Parameters>(++it, params, move(funcTupleText));
		case TypeContainer::TEXT_TUPLE:
			return funcTupleText(++it, params);
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	}

private:
	template <class Parameters>
	Dictionary parseFunctionDictionary(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleRegular, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
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
				dict.addSafe(move(name), parseFunctionListRegular<Parameters>(++it, params, move(funcTupleRegular), move(funcTupleText)));
				break;
			case TypeContainer::TUPLE:
				dict.addSafe(move(name), funcTupleRegular(++it, params));
				break;
			case TypeContainer::TEXT_LIST:
				dict.addSafe(move(name), parseFunctionListText<Parameters>(++it, params, move(funcTupleText)));
				break;
			case TypeContainer::TEXT_TUPLE:
				dict.addSafe(move(name), funcTupleText(++it, params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		} while (checkNextElementContainer(it, CON));
		return dict;
	}

	template <class Parameters>
	List parseFunctionListRegular(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleRegular, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
	{
		static const ConType CON = ConType::LIST;
		List list;
		if (checkEmptyContainer(it, CON))
			return list;
		do
			switch (skipJunkToContainer(it))
			{
			case TypeContainer::TUPLE:
				list.add(funcTupleRegular(++it, params));
				break;
			case TypeContainer::TEXT_TUPLE:
				list.add(funcTupleText(++it, params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		while (checkNextElementContainer(it, CON)); //make it so separators are not required (no need to clean line)
		return list;
	}

	template <class Parameters>
	List parseFunctionListText(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
	{
		static const ConType CON = ConType::LIST;
		List list;
		if (checkEmptyContainer(it, CON))
			return list;
		do
			switch (skipJunkToContainer(it))
			{
			case TypeContainer::TUPLE: case TypeContainer::TEXT_TUPLE:
				list.add(funcTupleText(++it, params));
				break;
			default:
				throw runtime_error(ERROR_UNEXPECTED);
			}
		while (checkNextElementContainer(it, CON)); //make it so separators are not required (no need to clean line)
		return list;
	}

	Webss parseFunctionContainer(It& it, const ParamStandard& defaultValue)
	{
		static const ConType CON = ConType::TUPLE;
		using Type = ParamStandard::TypeFhead;
		switch (defaultValue.getTypeFhead())
		{
		case Type::BINARY:
			return parseFunctionBodyBinary(it, defaultValue.getFunctionHeadBinary().getParameters());
		//case Type::SCOPED:
			//...
		case Type::STANDARD:
		{
			const auto& params = defaultValue.getFunctionHeadStandard().getParameters();
			return params.isText() ? parseFunctionBodyText(it, params) : parseFunctionBodyStandard(it, params);
		}
		default:
			return parseValueEqual(it, CON);
		}
	}

	Tuple parseFunctionTupleStandard(It& it, const FunctionHeadStandard::Tuple& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys());
		Tuple::size_type index = 0;
		if (!checkEmptyContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }))
			do
			{
				switch (*it)
				{
				case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
					tuple.at(index) = parseFunctionContainer(it, params.at(index));
					break;
				case CHAR_SELF:
				{
					if (!isNameStart(*skipJunkToValid(it)))
					{
						switch (*it)
						{
						case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
							tuple.at(index) = parseFunctionBodyStandard(it, params);
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
							tuple.at(name) = parseFunctionBodyStandard(it, params);
							break;
						default:
							throw runtime_error(ERROR_UNEXPECTED);
						}
					}
					break;
				}
				default:
					if (isNameStart(*it))
					{
						auto nameType = parseNameType(it);
						switch (nameType.type)
						{
						case NameType::NAME:
							switch (*skipJunkToValid(it))
							{
							case OPEN_DICTIONARY: case OPEN_LIST: case OPEN_TUPLE: case CHAR_COLON:
								tuple.at(nameType.name) = parseFunctionContainer(it, params.at(nameType.name));
								break;
							default:
								if (params.at(index).hasFunctionHead())
									throw runtime_error(ERROR_UNEXPECTED);
								tuple.at(nameType.name) = parseCharValue(it, CON);
								break;
							}
							break;
						case NameType::KEYWORD:
							if (params.at(index).hasFunctionHead())
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(nameType.keyword);
							break;
						case NameType::ENTITY:
						{
							if (params.at(index).hasFunctionHead())
								throw runtime_error(ERROR_UNEXPECTED);
							auto otherValue = checkOtherValueEntity(it, CON, nameType.entity);
							if (otherValue.type != OtherValue::VALUE_ONLY)
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(otherValue.value);
							break;
						}
						default:
							throw domain_error("");
						}
					}
					else
					{
						parseOtherValue(it, CON,
							ErrorKeyValue(ERROR_UNEXPECTED),
							ErrorKeyOnly(webss_ERROR_UNDEFINED_KEYNAME(key)),
							CaseValueOnly
						{
							if (params.at(index).hasFunctionHead())
							throw runtime_error(ERROR_UNEXPECTED);
						tuple.at(index) = move(value);
						},
							ErrorAbstractEntity(ERROR_UNEXPECTED));
					}
					break;
				}
				++index;
			} while (checkNextElementContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }));
			checkDefaultValues(tuple, params);
			return tuple;
	}

	Tuple parseFunctionTupleText(It& it, const FunctionHeadStandard::Tuple& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys(), params.isText());
		Tuple::size_type index = 0;
		if (!checkEmptyContainerVoid(it, CON, [&]() { ++index; }))
			do
				tuple.at(index++) = parseLineString(it, CON);
		while (checkNextElementContainerVoid(it, CON, [&]() { ++index; }));
		checkDefaultValues(tuple, params);
		return tuple;
	}
};

Webss Parser::parseFunction(It& it, ConType con)
{
	using Type = FunctionHeadSwitch::Type;
	auto headSwitch = parseFunctionHead(it);
	switch (headSwitch.t)
	{
	case Type::BLOCK:
		return Block(move(headSwitch.blockHead), parseValueEqual(it, con));
	case Type::BINARY:
	{
		auto head = move(headSwitch.fheadBinary);
		auto body = parseFunctionBodyBinary(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case Type::SCOPED:
		//...
	case Type::STANDARD:
	{
		auto head = move(headSwitch.fheadStandard);
		auto body = parseFunctionBodyStandard(it, head.getParameters());
		return{ move(head), move(body) };
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

Webss Parser::parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Tuple& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyStandard(it, params);
}

Webss Parser::parseFunctionBodyText(It& it, const FunctionHeadStandard::Tuple& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyText(it, params);
}

Webss Parser::parseFunctionBodyBinary(It& it, const FunctionHeadBinary::Tuple& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBody<FunctionHeadBinary::Tuple>(it, params, [&](It& it, const FunctionHeadBinary::Tuple& params) { return parseFunctionTupleBinary(it, params); }, [&](It& it, const FunctionHeadBinary::Tuple& params) -> Tuple { throw runtime_error(ERROR_UNEXPECTED); });
}