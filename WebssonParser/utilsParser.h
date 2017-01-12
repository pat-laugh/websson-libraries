//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonStructures/webss.h"

namespace webss
{
	//parserFunctions
	void setDefaultValue(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index);
	//parserFunctions
	void setDefaultValueSafe(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index);
	//parserFunctions
	Tuple makeDefaultTuple(const FunctionHeadStandard::Tuple& defaultTuple);

	//parsers
	Webss&& checkIsConcrete(Webss&& webss);
	const Webss& checkIsConcrete(const Webss& webss);
	const BasicVariable<Webss>& checkIsConcrete(const BasicVariable<Webss>& var);
	const FunctionHeadStandard& checkIsFunctionHeadStandard(const Webss& webss);
	const FunctionHeadBinary& checkIsFunctionHeadBinary(const Webss& webss);
}