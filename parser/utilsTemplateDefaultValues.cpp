//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsTemplateDefaultValues.hpp"

#include "structures/paramBinary.hpp"
#include "structures/paramStandard.hpp"
#include "structures/tuple.hpp"

using namespace std;
using namespace webss;

const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";

void webss::setDefaultValue(Webss& value, const ParamBinary& defaultValue)
{
	if (defaultValue.hasDefaultValue())
		value = Webss(defaultValue.getDefaultPointer());
	else if (defaultValue.isTemplateHeadBinary())
		value = makeDefaultTuple(defaultValue.getTemplateHead().getParameters());
	else
		throw runtime_error(ERROR_NO_DEFAULT);
}

void webss::setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasDefaultValue())
		value = Webss(defaultValue.getDefaultPointer());
	else
	{
		switch (defaultValue.getTypeThead())
		{
		case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
			value = makeDefaultTuple(defaultValue.getTemplateHeadBinary().getParameters());
			break;
		case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
			value = makeDefaultTuple(defaultValue.getTemplateHeadStandard().getParameters());
			break;
		default:
			throw runtime_error(ERROR_NO_DEFAULT);
		}
	}
}

void webss::checkDefaultValues(Tuple& tuple, const TemplateHeadStandard::Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}