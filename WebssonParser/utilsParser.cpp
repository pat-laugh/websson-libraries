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

void webss::setDefaultValue(Tuple& tuple, const FunctionHeadStandard::Tuple& params, Tuple::size_type index)
{
	::setDefaultValue(tuple[index], params[index]);
}

Tuple webss::makeDefaultTuple(const FunctionHeadStandard::Tuple& params)
{
	Tuple tuple(params.getSharedKeys(), params.isText());
	for (Tuple::size_type i = 0; i < params.size(); ++i)
		setDefaultValue(tuple, params, i);
	return tuple;
}

const Namespace& webss::checkIsNamespace(const BasicEntity<Webss>& ent)
{
	try { return ent.getContent().getNamespace(); }
	catch (exception e) { throw runtime_error("expected a namespace"); }
}