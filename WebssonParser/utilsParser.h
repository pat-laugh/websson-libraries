//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonStructures/webss.h"

namespace webss
{
	const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";

	void setDefaultValue(Webss& value, const ParamStandard& defaultValue);
	void setDefaultValue(Webss& value, const ParamText& defaultValue);

	template <class Parameters>
	Tuple makeDefaultTuple(const Parameters& params)
	{
		Tuple tuple(params.getSharedKeys());
		for (Tuple::size_type i = 0; i < params.size(); ++i)
			setDefaultValue(tuple[i], params[i]);
		return tuple;
	}
}