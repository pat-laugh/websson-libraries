//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "structures/webss.hpp"
#include "structures/templateHead.hpp"

namespace webss
{
	void setDefaultValue(Webss& value, const ParamBinary& defaultValue);
	void setDefaultValue(Webss& value, const ParamStandard& defaultValue);
	void checkDefaultValues(Tuple& tuple, const TemplateHeadStandard::Parameters& params);

	template <class Parameters>
	Tuple makeDefaultTuple(const Parameters& params)
	{
		Tuple tuple(params.getSharedKeys());
		for (Tuple::size_type i = 0; i < params.size(); ++i)
			setDefaultValue(tuple[i], params[i]);
		return tuple;
	}
}