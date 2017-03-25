//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "WebssonStructures/webss.h"
#include "WebssonUtils/conType.h"
#include "WebssonUtils/iterators.h"

namespace webss
{
	const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";

	void setDefaultValue(Webss& value, const ParamStandard& defaultValue);

	template <class Parameters>
	Tuple makeDefaultTuple(const Parameters& params)
	{
		Tuple tuple(params.getSharedKeys());
		for (Tuple::size_type i = 0; i < params.size(); ++i)
			setDefaultValue(tuple[i], params[i]);
		return tuple;
	}

	bool checkContainerEnd(SmartIterator& it, ConType con);
}