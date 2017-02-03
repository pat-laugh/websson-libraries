//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"
#include "utilsParser.h"

using namespace std;
using namespace webss;

template <class Parameters>
void checkDefaultValues(Tuple& tuple, const Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).type == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}

class ParserFunctions : public Parser
{
public:
	Webss parseFunctionBodyScoped(It& it, const FunctionHeadScoped::Parameters& params, ConType con)
	{
		ParamDocumentIncluder includer(ents, params);
		return parseValueOnly(it, con);
	}

	Webss parseFunctionBodyStandard(It& it, const FunctionHeadStandard::Parameters& params)
	{
		return parseFunctionBody<FunctionHeadStandard::Parameters>(it, params, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleStandard(it, params); }, [&](It& it, const FunctionHeadStandard::Parameters& params) { return parseFunctionTupleText(it, params); });
	}

	Webss parseFunctionBodyText(It& it, const FunctionHeadText::Parameters& params)
	{
		return parseFunctionBody<FunctionHeadText::Parameters>(it, params, [&](It& it, const FunctionHeadText::Parameters& params) { return parseFunctionTupleText(it, params); }, [&](It& it, const FunctionHeadText::Parameters& params) { return parseFunctionTupleText(it, params); });
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
		return parseContainer<Dictionary, ConType::DICTIONARY>(it, Dictionary(), [&](Dictionary& dict, ConType con)
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
		});
	}

	template <class Parameters>
	List parseFunctionListRegular(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleRegular, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
	{
		return parseContainer<List, ConType::LIST>(it, List(), [&](List& list, ConType con)
		{
			auto nextCont = skipJunkToContainer(it);
			if (nextCont == TypeContainer::TUPLE)
				list.add(funcTupleRegular(++it, params));
			else if (nextCont == TypeContainer::TEXT_TUPLE)
				list.add(funcTupleText(++it, params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	template <class Parameters>
	List parseFunctionListText(It& it, const Parameters& params, function<Tuple(It& it, const Parameters& params)>&& funcTupleText)
	{
		return parseContainer<List, ConType::LIST>(it, List(true), [&](List& list, ConType con)
		{
			auto nextCont = skipJunkToContainer(it);
			if (nextCont == TypeContainer::TUPLE || nextCont == TypeContainer::TEXT_TUPLE)
				list.add(funcTupleText(++it, params));
			else
				throw runtime_error(ERROR_UNEXPECTED);
		});
	}

	Webss parseFunctionContainer(It& it, const FunctionHeadStandard::Parameters& params, const ParamStandard& defaultValue)
	{
		static const ConType CON = ConType::TUPLE;
		switch (defaultValue.getTypeFhead())
		{
		case WebssType::FUNCTION_HEAD_BINARY:
			return parseFunctionBodyBinary(it, defaultValue.getFunctionHeadBinary().getParameters());
		case WebssType::FUNCTION_HEAD_SCOPED:
			return parseFunctionBodyScoped(it, defaultValue.getFunctionHeadScoped().getParameters(), CON);
		case WebssType::FUNCTION_HEAD_SELF:
			return parseFunctionBodyStandard(it, params);
		case WebssType::FUNCTION_HEAD_STANDARD:
			return parseFunctionBodyStandard(it, defaultValue.getFunctionHeadStandard().getParameters());
		case WebssType::FUNCTION_HEAD_TEXT:
			return parseFunctionBodyText(it, defaultValue.getFunctionHeadText().getParameters());
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
		{
			do
			{
				if (!isNameStart(*it))
					tuple.at(index) = parseFunctionContainer(it, params, params.at(index));
				else
				{
					auto nameType = parseNameType(it);
					if (nameType.type == NameType::NAME)
						tuple.at(nameType.name) = parseFunctionContainer(it, params, params.at(nameType.name));
					else
					{
						if (params.at(index).hasFunctionHead())
							throw runtime_error(ERROR_UNEXPECTED);
						switch (nameType.type)
						{
						case NameType::KEYWORD:
							tuple.at(index) = move(nameType.keyword);
							break;
						case NameType::ENTITY_ABSTRACT:
						{
							auto otherValue = checkAbstractEntity(it, CON, nameType.entity);
							if (otherValue.type != OtherValue::VALUE_ONLY)
								throw runtime_error(ERROR_UNEXPECTED);
							tuple.at(index) = move(otherValue.value);
							break;
						}
						case NameType::ENTITY_CONCRETE:
							tuple.at(index) = move(nameType.entity);
							break;
						default:
							assert(false);
						}
					}
					
				}
				++index;
			} while (checkNextElementContainerVoid(it, CON, [&]() { if (index++ >= tuple.size()) throw runtime_error("too many values"); }));
		}
		checkDefaultValues(tuple, params);
		return tuple;
	}

	template <class Parameters>
	Tuple parseFunctionTupleText(It& it, const Parameters& params)
	{
		static const ConType CON = ConType::TUPLE;
		Tuple tuple(params.getSharedKeys(), true);
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
	switch (headWebss.type)
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
	case WebssType::FUNCTION_HEAD_SELF:
		throw runtime_error("self in a fhead must be within a non-empty fhead");
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

Webss Parser::parseFunctionBodyText(It& it, const FunctionHeadText::Parameters& params)
{
	return static_cast<ParserFunctions*>(this)->parseFunctionBodyText(it, params);
}