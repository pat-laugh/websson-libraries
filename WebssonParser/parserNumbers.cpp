//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "utilsNumbers.h"

using namespace std;
using namespace webss;

Webss Parser::parseNumber(It& it)
{
	bool negative = checkNumberNegative(it);
	auto base = checkNumberBase(it);
	auto num = parseInt(it, base);
	if (negative)
		num = -num;

	if (!it || isNumberEnd(*it, language))
		return num;

	double numDouble = (double)num;

	if (isDecimalSeparator(*it, language))
	{
		if (!skipLineJunk(++it))
			throw runtime_error(ERROR_EXPECTED_NUMBER);

		auto decimals = getDecimals(it, base);
		if (negative)
			decimals = -decimals;

		numDouble += decimals;

		if (!it || isNumberEnd(*it, language))
			return numDouble;

		if (!isBaseSeparator(*it))
			throw runtime_error("invalid number");
	}

	if (!skipLineJunk(++it) || !isNumberStart(*it))
		throw runtime_error(ERROR_EXPECTED_NUMBER);
	return addNumberBase(it, numDouble, base);
}