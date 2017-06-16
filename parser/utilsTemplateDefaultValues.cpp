//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsTemplateDefaultValues.hpp"

#include "structures/paramBinary.hpp"
#include "structures/paramStandard.hpp"
#include "structures/tuple.hpp"

using namespace std;
using namespace webss;

const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";

void webss::setDefaultValue(Webss& value, const ParamBin& defaultValue)
{
	if (defaultValue.hasDefaultValue())
		value = Webss(defaultValue.getDefaultPointer());
	else if (defaultValue.isTheadBin())
		value = makeDefaultTuple(defaultValue.getThead().getParams());
	else
		throw runtime_error(ERROR_NO_DEFAULT);
}

void webss::setDefaultValue(Webss& value, const ParamStd& param)
{
	if (param.hasDefaultValue())
		value = Webss(param.getDefaultPointer());
	else if (!param.hasThead())
		throw runtime_error(ERROR_NO_DEFAULT);
	else if (param.getTypeThead() == TypeThead::BIN)
		value = makeDefaultTuple(param.getTheadBin().getParams());
	else
	{
		assert(param.getTypeThead() == TypeThead::STD);
		value = makeDefaultTuple(param.getTheadStd().getParams());
	}
}

void webss::checkDefaultValues(Tuple& tuple, const TheadStd::Params& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}