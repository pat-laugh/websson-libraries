//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

void checkDefaultValues(Tuple& tuple, const FunctionHeadStandard::Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).t == WebssType::NONE)
			setDefaultValue(tuple, params, index);
}

class ParserFunctions : public Parser
{
public:
	Webss parseFunctionBodyScoped(It& it, const FunctionHeadScoped::Parameters& params, ConType con)
	{
		ParamDocumentIncluder includer(static_cast<vector<ParamDocument>>(params));
		return parseValueOnly(it, con);
	}

	Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Parameters& params)
	{
		return parseFunctionBody<FunctionHeadStandard::Parameters>(it, params, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleStandard(it, params); }, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleText(it, params); });
	}

	Webss parseFunctionBodyText(It& it, const FunctionHeadStandard::Parameters& params)
	{
		return parseFunctionBody<FunctionHeadStandard::Parameters>(it, params, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleText(it, params); }, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleText(it, params); });
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
		switch (defaultValue.getTypeFhead())
		{
		case WebssType::FUNCTION_HEAD_BINARY:
			return parseFunctionBodyBinary(it, defaultValue.getFunctionHeadBinary().getParameters());
		case WebssType::FUNCTION_HEAD_SCOPED:
			return parseFunctionBodyScoped(it, defaultValue.getFunctionHeadScoped().getParameters(), CON);
		case WebssType::FUNCTION_HEAD_STANDARD:
		{
			const auto& params = defaultValue.getFunctionHeadStandard().getParameters();
			return params.isText() ? parseFunctionBodyText(it, params) : parseFunctionBodyStandard(it, params);
		}
		default:
			return parseValueOnly(it, CON);
		}
	}

	Tuple parseFunctionTupleStandard(It& it, const FunctionHeadStandard::Parameters& params)
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
					auto head = FunctionHeadStandard(params);
					if (!isNameStart(*skipJunkToValid(it)))
						tuple.at(index) = { move(head), parseFunctionBodyStandard(it, params) };
					else
					{
						auto name = parseNameSafe(it);
						tuple.at(name) = { move(head), parseFunctionBodyStandard(it, params) };
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
							tuple.at(nameType.name) = parseFunctionContainer(it, params.at(nameType.name));
							break;
						case NameType::KEYWORD:
							if (params.at(index).hasFunctionHead())
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(nameType.keyword);
							break;
						case NameType::ENTITY:
						{
							auto otherValue = checkOtherValueEntity(it, CON, nameType.entity);
							if (params.at(index).hasFunctionHead() || otherValue.type != OtherValue::VALUE_ONLY)
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(otherValue.value);
							break;
						}
						default:
							assert(false);
						}
					}
					else
					{
						if (params.at(index).hasFunctionHead())
							throw runtime_error(ERROR_UNEXPECTED);
						tuple.at(index) = parseValueOnly(it, CON);
					}
					break;
				}
				++index;
			} while (checkNextElementContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }));
			checkDefaultValues(tuple, params);
			return tuple;
	}

	Tuple parseFunctionTupleText(It& it, const FunctionHeadStandard::Parameters& params)
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
	auto headWebss = parseFunctionHead(it);
	switch (headWebss.t)
	{
	case WebssType::BLOCK_HEAD:
		return Block(move(*headWebss.blockHead), parseValueOnly(it, con));
	case WebssType::FUNCTION_HEAD_BINARY:
	{
		auto head = move(*headWebss.fheadBinary);
		auto body = parseFunctionBodyBinary(it, head.getParameters());
		return{ move(head), move(body) };
	}
	case WebssType::FUNCTION_HEAD_SCOPED:
	{
		auto head = move(*headWebss.fheadScoped);
		auto body = parseFunctionBodyScoped(it, head.getParameters(), con);
		return FunctionScoped(move(head), move(body));
	}
	case WebssType::FUNCTION_HEAD_STANDARD:
	{
		auto head = move(*headWebss.fheadStandard);
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

Webss Parser::parseFunctionBodyBinary(It& it, const FunctionHeadBinary::Parameters& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBody<FunctionHeadBinary::Parameters>(it, params, [&](It& it, const FunctionHeadBinary::Parameters& params) { return parseFunctionTupleBinary(it, params); }, [&](It& it, const FunctionHeadBinary::Parameters& params) -> Tuple { throw runtime_error(ERROR_UNEXPECTED); });
}

Webss Parser::parseFunctionBodyScoped(It& it, const FunctionHeadScoped::Parameters& params, ConType con)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyScoped(it, params, con);
}

Webss Parser::parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Parameters& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyStandard(it, params);
}

Webss Parser::parseFunctionBodyText(It& it, const FunctionHeadStandard::Parameters& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyText(it, params);
}