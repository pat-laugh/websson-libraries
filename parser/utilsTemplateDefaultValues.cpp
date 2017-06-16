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

void webss::setDefaultValue(Webss& value, const ParamStandard& param)
{
	if (param.hasDefaultValue())
		value = Webss(param.getDefaultPointer());
	else if (!param.hasThead())
		throw runtime_error(ERROR_NO_DEFAULT);
	else if (param.getTypeThead() == TypeThead::BINARY)
		value = makeDefaultTuple(param.getTheadBin().getParameters());
	else
	{
		assert(param.getTypeThead() == TypeThead::STANDARD);
		value = makeDefaultTuple(param.getTheadStd().getParameters());
	}
}

void webss::checkDefaultValues(Tuple& tuple, const TemplateHeadStandard::Parameters& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}