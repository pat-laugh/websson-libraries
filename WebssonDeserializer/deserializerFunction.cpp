//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "deserializer.h"

using namespace std;
using namespace webss;

void Deserializer::putFuncBinary(StringBuilder& out, const TemplateBinary& templ)
{
	putFheadBinary(out, templ);
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		putFuncBinaryDictionary(out, templ.getParameters(), templ.getDictionary());
		break;
	case WebssType::LIST:
		putFuncBinaryList(out, templ.getParameters(), templ.getList());
		break;
	case WebssType::TUPLE:
		putFuncBinaryTuple(out, templ.getParameters(), templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncStandard(StringBuilder& out, const TemplateStandard& templ)
{
	putFheadStandard(out, templ);
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
	default:
		assert(false); throw domain_error("");
	}
}

void Deserializer::putFuncText(StringBuilder& out, const TemplateText& templ)
{
	putFheadText(out, templ);
	switch (templ.getType())
	{
	case WebssType::DICTIONARY:
		putFuncTextDictionary(out, templ.getParameters(), templ.getDictionary());
		break;
	case WebssType::LIST:
		putFuncTextList(out, templ.getParameters(), templ.getList());
		break;
	case WebssType::TUPLE:
		putFuncTextTuple(out, templ.getParameters(), templ.getTuple());
		break;
	default:
		assert(false); throw domain_error("");
	}
}