//MIT License
//Copyright 2017 Patrick Laughrea
#include "utilsTemplateDefaultValues.hpp"

#include "structures/paramBinary.hpp"
#include "structures/paramStandard.hpp"
#include "structures/thead.hpp"

using namespace std;
using namespace webss;

static const char* ERROR_NO_DEFAULT = "no default value, so value must be implemented";

Tuple webss::makeDefaultTuple(const Thead& thead)
{
	assert(thead.isTheadBin() || thead.isTheadStd());
	if (thead.isTheadBin())
		return makeDefaultTuple(thead.getTheadBin().getParams());
	else
		return makeDefaultTuple(thead.getTheadStd().getParams());
}

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
	else if (param.hasThead())
		value = makeDefaultTuple(param.getThead());
	else
		throw runtime_error(ERROR_NO_DEFAULT);
}

void webss::checkDefaultValues(Tuple& tuple, const TheadStd::Params& params)
{
	for (Tuple::size_type index = 0; index < tuple.size(); ++index)
		if (tuple.at(index).getTypeRaw() == WebssType::NONE)
			setDefaultValue(tuple[index], params[index]);
}