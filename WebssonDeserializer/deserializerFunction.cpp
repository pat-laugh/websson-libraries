//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void Deserializer::putFuncBinary(StringBuilder& out, const FunctionBinary& func)
{
	putFheadBinary(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncBinaryDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncBinaryList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncBinaryTuple(out, func.getParameters(), func.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncStandard(StringBuilder& out, const FunctionStandard& func)
{
	putFheadStandard(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncStandardDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncStandardList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncStandardTuple(out, func.getParameters(), func.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncText(StringBuilder& out, const FunctionText& func)
{
	putFheadText(out, func);
	switch (func.getType())
	{
	case WebssType::DICTIONARY:
		putFuncTextDictionary(out, func.getParameters(), func.getDictionary());
		break;
	case WebssType::LIST:
		putFuncTextList(out, func.getParameters(), func.getList());
		break;
	case WebssType::TUPLE:
		putFuncTextTuple(out, func.getParameters(), func.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}