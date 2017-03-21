//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "utilsParser.h"

#include "utilsSweepers.h"
#include "errors.h"

using namespace std;
using namespace webss;

void webss::setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasTemplateHead())
		value = makeDefaultTuple(defaultValue.getTemplateHeadStandard().getParameters());
	else if (!defaultValue.hasDefaultValue())
		throw runtime_error(ERROR_NO_DEFAULT);
	else
		value = Webss(defaultValue.getDefaultPointer());
}

bool webss::checkContainerEnd(SmartIterator& it, ConType con)
{
	if (!skipJunk(it))
	{
		if (con.hasEndChar())
			throw runtime_error(ERROR_EXPECTED);
		return true;
	}
	if (con.isEnd(*it))
	{
		++it;
		return true;
	}
	return false;
}