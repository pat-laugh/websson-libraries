//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonStructures/webss.h"

namespace webss
{
	//parserFunctions
	void setDefaultValue(Tuple& tuple, const FunctionHeadStandard::Parameters& params, FunctionHeadStandard::Parameters::size_type index);
	//parserFunctions
	Tuple makeDefaultTuple(const FunctionHeadStandard::Parameters& params);

	//parsers
	const Namespace& checkIsNamespace(const BasicEntity<Webss>& ent);
}