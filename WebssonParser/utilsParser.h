//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonStructures/webss.h"

namespace webss
{
	//parserFunctions
	void setDefaultValue(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index);
	//parserFunctions
	Tuple makeDefaultTuple(const FunctionHeadStandard::Tuple& defaultTuple);

	//parsers
	const Namespace& checkIsNamespace(const BasicEntity<Webss>& ent);
}