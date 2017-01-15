//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "utilsParser.h"

using namespace std;
using namespace webss;

const char ERROR_NO_DEFAULT[] = "no default value, so value must be implemented";
const char ERROR_BINARY_SIZE_HEAD[] = "size of binary head must be a positive integer, binary function head or equivalent entity";
const char ERROR_BINARY_SIZE_LIST[] = "size of binary list must be a positive integer or equivalent entity";

void setDefaultValue(Webss& value, const ParamStandard& defaultValue);

void setDefaultValue(Webss& value, const ParamStandard& defaultValue)
{
	if (defaultValue.hasFunctionHead())
		value = makeDefaultTuple(defaultValue.getFunctionHeadStandard().getParameters());
	else if (!defaultValue.hasDefaultValue())
		throw runtime_error(ERROR_NO_DEFAULT);
	else
		value = Webss(defaultValue.getDefaultPointer());
}

void webss::setDefaultValue(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index)
{
	::setDefaultValue(tuple[index], defaultTuple[index]);
}

void webss::setDefaultValueSafe(Tuple& tuple, const FunctionHeadStandard::Tuple& defaultTuple, Tuple::size_type index)
{
	::setDefaultValue(tuple.at(index), defaultTuple.at(index));
}

Tuple webss::makeDefaultTuple(const FunctionHeadStandard::Tuple& defaultTuple)
{
	Tuple tuple(defaultTuple.getSharedKeys(), defaultTuple.containerText);
	for (Tuple::size_type i = 0; i < defaultTuple.size(); ++i)
		setDefaultValue(tuple, defaultTuple, i);
	return tuple;
}



const char ERROR_IS_FUNCTION_HEAD[] = "unexpected value of type function head";
const char ERROR_IS_NOT_FUNCTION_HEAD[] = "expected value of type function head";

Webss&& webss::checkIsConcrete(Webss&& webss)
{
	if (!webss.isConcrete())
		throw runtime_error("expected a concrete value");
	return move(webss);
}

const Webss& webss::checkIsConcrete(const Webss& webss)
{
	if (!webss.isConcrete())
		throw runtime_error("expected a concrete value");
	return webss;
}

const BasicEntity<Webss>& webss::checkIsConcrete(const BasicEntity<Webss>& ent)
{
	if (!ent.getContent().isConcrete())
		throw runtime_error("expected a concrete value");
	return ent;
}

const Namespace& webss::checkIsNamespace(const BasicEntity<Webss>& ent)
{
	try { return ent.getContent().getNamespace(); }
	catch (exception e) { throw runtime_error("expected a namespace"); }
}