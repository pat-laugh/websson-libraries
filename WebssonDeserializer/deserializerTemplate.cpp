//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void Deserializer::putFuncBinary(StringBuilder& out, const TemplateBinary& templ)
{
	putTheadBinary(out, templ);
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		putFuncBinaryDictionary(out, templ.getParameters(), templ.getDictionary());
		break;
	case WebssType::LIST:
		putFuncBinaryList(out, templ.getParameters(), templ.getList());
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putFuncBinaryTuple(out, templ.getParameters(), templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncStandard(StringBuilder& out, const TemplateStandard& templ)
{
	putTheadStandard(out, templ);
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		putFuncStandardDictionary(out, templ.getParameters(), templ.getDictionary());
		break;
	case WebssType::LIST:
		putFuncStandardList(out, templ.getParameters(), templ.getList());
		break;
	case WebssType::TUPLE:
		putFuncStandardTuple(out, templ.getParameters(), templ.getTuple());
		break;
	case WebssType::TUPLE_TEXT:
		putFuncStandardTupleText(out, templ.getParameters(), templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncText(StringBuilder& out, const TemplateStandard& templ)
{
	putTheadText(out, templ);
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		putFuncTextDictionary(out, templ.getParameters(), templ.getDictionary());
		break;
	case WebssType::LIST:
		putFuncTextList(out, templ.getParameters(), templ.getList());
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putFuncTextTuple(out, templ.getParameters(), templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}