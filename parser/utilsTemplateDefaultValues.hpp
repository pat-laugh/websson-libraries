//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "structures/base.hpp"
#include "structures/templateHead.hpp"
#include "structures/tuple.hpp"

namespace webss
{
	Tuple makeDefaultTuple(const Thead& thead);
	void setDefaultValue(Webss& value, const ParamBin& defaultValue);
	void setDefaultValue(Webss& value, const ParamStd& defaultValue);
	void checkDefaultValues(Tuple& tuple, const TheadStd::Params& params);

	template <class Params>
	Tuple makeDefaultTuple(const Params& params)
	{
		Tuple tuple(params.getSharedKeys());
		for (Tuple::size_type i = 0; i < params.size(); ++i)
			setDefaultValue(tuple[i], params[i]);
		return tuple;
	}
}