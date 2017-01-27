//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsParser.h"

using namespace std;
using namespace webss;

void webss::setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasFunctionHead())
		value = makeDefaultTuple(defaultValue.getFunctionHeadStandard().getParameters());
	else if (!defaultValue.hasDefaultValue())
		throw std::runtime_error(ERROR_NO_DEFAULT);
	else
		value = Webss(defaultValue.getDefaultPointer());
}

void webss::setDefaultValue(Webss& value, const ParamText& defaultValue)
{
	if (!defaultValue.hasDefaultValue())
		throw std::runtime_error(ERROR_NO_DEFAULT);
	else
		value = Webss(defaultValue.getDefaultPointer());
}